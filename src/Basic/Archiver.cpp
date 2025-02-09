//===-- llvm-ar.cpp - LLVM archive librarian utility ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Builds up (relatively) standard unix archive files (.a) containing LLVM
// bitcode or other files.
//
//===----------------------------------------------------------------------===//

#include "Basic/Archiver.h"
#include "Basic/Debug.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/Triple.h"
#include "llvm/BinaryFormat/Magic.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Object/IRObjectFile.h"
#include "llvm/Object/MachO.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Object/SymbolicFile.h"
#include "llvm/Support/Chrono.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ConvertUTF.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/StringSaver.h"
#include "llvm/Support/raw_ostream.h"

#if !defined(_MSC_VER) && !defined(__MINGW32__)
#include <unistd.h>
#else
#include <io.h>
#endif

using namespace llvm;
using namespace fly;

Archiver::Archiver(DiagnosticsEngine &Diag, const std::string &ArchiveName) : Diag(Diag), ArchiveName(ArchiveName) {
    FLY_DEBUG_MESSAGE("Archiver", "Archiver", "ArchiveName=" + ArchiveName);
}

bool Archiver::CreateLib(const llvm::SmallVector<std::string, 4> &Files) {
    for (auto &File : Files) {
        FLY_DEBUG_MESSAGE("Archiver", "CreateLib", "File=" << File);
        // Everything on the command line at this point is a member.
        Members.emplace_back(File);
    }
    // Create or open the archive object.
    ErrorOr<std::unique_ptr<MemoryBuffer>> Buf =
            MemoryBuffer::getFile(ArchiveName, -1, false);
    std::error_code EC = Buf.getError();
    if (EC && EC == errc::no_such_file_or_directory) {
        return performWriteOperation(ReplaceOrInsert, nullptr, nullptr, nullptr);
    }
    return fail("File error: '" + ArchiveName + "' already exists " + EC.message());
}

bool Archiver::ExtractLib(FileManager &FileMgr) {
    FLY_DEBUG_START("Archiver", "ExtractLib");
    // Create or open the archive object.
    ErrorOr<std::unique_ptr<MemoryBuffer>> Buf =
            MemoryBuffer::getFile(ArchiveName, -1, false);
    std::error_code EC = Buf.getError();
    std::vector<StringRef> HeaderFiles;
    if (EC) {
        return fail("unable to open '" + ArchiveName + "': " + EC.message());
    }

    Error Err = Error::success();
    object::Archive Arch(Buf.get()->getMemBufferRef(), Err);
    if (isError(std::move(Err), "unable to load '" + ArchiveName + "'")) {
        return false;
    }
    return performReadOperation(Extract, &Arch);
}

const std::vector<StringRef> &Archiver::getExtractFiles() const {
    return ExtractFiles;
}

bool Archiver::fail(Twine Error) {
    Diag.Report(diag::err_drv_archive) << Error.str();
    return false;
}

bool Archiver::isError(std::error_code EC, Twine Context) {
    if (EC) {
        std::string ContextStr = Context.str();
        if (ContextStr.empty())
            Diag.Report(diag::err_drv_archive) << EC.message();
        Diag.Report(diag::err_drv_archive) << ContextStr + ":" + EC.message();
        return true;
    }
    return false;
}

bool Archiver::isError(Error E, Twine Context) {
    bool IsError = false;
    handleAllErrors(std::move(E), [&](const llvm::ErrorInfoBase &EIB) {
        std::string ContextStr = Context.str();
        if (ContextStr.empty())
            fail(EIB.message());
        fail(ContextStr + ":" + EIB.message());
        IsError = true;
    });
    return IsError;
}

object::Archive *Archiver::readLibrary(const Twine &Library) {
    auto BufOrErr = MemoryBuffer::getFile(Library, -1, false);
    if (isError(BufOrErr.getError(), "could not open library " + Library)) {
        return nullptr;
    }
    std::vector<std::unique_ptr<MemoryBuffer>> ArchiveBuffers;
    ArchiveBuffers.push_back(std::move(*BufOrErr));
    auto LibOrErr = object::Archive::create(ArchiveBuffers.back()->getMemBufferRef());
    if (isError(errorToErrorCode(LibOrErr.takeError()), "could not parse library")) {
        return nullptr;
    }

    std::vector<std::unique_ptr<object::Archive>> Archives;
    Archives.push_back(std::move(*LibOrErr));
    return &*Archives.back();
}

std::string Archiver::normalizePath(StringRef Path) {
    return CompareFullPath ? llvm::sys::path::convert_to_slash(Path) : std::string(llvm::sys::path::filename(Path));
}

bool Archiver::comparePaths(StringRef Path1, StringRef Path2) {
    return normalizePath(Path1) == normalizePath(Path2);
}

// Implement the 'x' operation. This function extracts files back to the file
// system.
bool Archiver::doExtract(StringRef Name, const object::Archive::Child &C) {
    // Retain the original mode.
    Expected<sys::fs::perms> ModeOrErr = C.getAccessMode();
    if (isError(ModeOrErr.takeError())) {
        return false;
    }
    sys::fs::perms Mode = ModeOrErr.get();

    llvm::StringRef outputFilePath = sys::path::filename(Name);
    ExtractFiles.push_back(outputFilePath);
    int FD;
    std::error_code EC = sys::fs::openFileForWrite(outputFilePath, FD,
                                                   sys::fs::CD_CreateAlways,
                                                   sys::fs::OF_None, Mode);
    if (isError(EC, Name))
        return false;

    {
        raw_fd_ostream file(FD, false);

        // Get the data and its length
        Expected<StringRef> BufOrErr = C.getBuffer();
        if (isError(BufOrErr.takeError()))
            return false;
        StringRef Data = BufOrErr.get();

        // Write the data.
        file.write(Data.data(), Data.size());
    }

    // If we're supposed to retain the original modification times, etc. do so
    // now.
    if (OriginalDates) {
        auto ModTimeOrErr = C.getLastModified();
        if (isError(ModTimeOrErr.takeError()))
            return false;
        if (isError(sys::fs::setLastAccessAndModificationTime(FD, ModTimeOrErr.get())))
            return false;
    }

    if (close(FD))
        return fail("Could not close the file");

    return true;
}

bool Archiver::addChildMember(std::vector<NewArchiveMember> &Members,
                              const object::Archive::Child &M,
                              bool FlattenArchive) {
    if (Thin && !M.getParent()->isThin())
        return fail("cannot convert a regular archive to a thin one");
    Expected<NewArchiveMember> NMOrErr =
            NewArchiveMember::getOldMember(M, Deterministic);
    if (isError(NMOrErr.takeError())) {
        return false;
    }
    // If the child member we're trying to add is thin, use the path relative to
    // the archive it's in, so the file resolves correctly.
    if (Thin && FlattenArchive) {
        StringSaver Saver(Alloc);
        Expected<std::string> FileNameOrErr(M.getName());
        if (isError(FileNameOrErr.takeError())) {
            return false;
        }
        if (sys::path::is_absolute(*FileNameOrErr)) {
            NMOrErr->MemberName = Saver.save(sys::path::convert_to_slash(*FileNameOrErr));
        } else {
            FileNameOrErr = M.getFullName();
            if (isError(FileNameOrErr.takeError())) {
                return false;
            }
            Expected<std::string> PathOrErr =
                    computeArchiveRelativePath(ArchiveName, *FileNameOrErr);
            NMOrErr->MemberName = Saver.save(
                    PathOrErr ? *PathOrErr : llvm::sys::path::convert_to_slash(*FileNameOrErr));
        }
    }
    if (FlattenArchive &&
        identify_magic(NMOrErr->Buf->getBuffer()) == file_magic::archive) {
        Expected<std::string> FileNameOrErr = M.getFullName();
        if (isError(FileNameOrErr.takeError())) {
            return false;
        }
        object::Archive *Lib = readLibrary(*FileNameOrErr);
        if (Lib == nullptr) return false;
        // When creating thin archives, only flatten if the member is also thin.
        if (!Thin || Lib->isThin()) {
            Error Err = Error::success();
            // Only Thin archives are recursively flattened.
            for (auto &Child: Lib->children(Err))
                addChildMember(Members, Child, /*FlattenArchive=*/Thin);
            if (isError(std::move(Err))) {
                return false;
            }
            return true;
        }
    }
    Members.push_back(std::move(*NMOrErr));
    return true;
}

bool Archiver::addMember(std::vector<NewArchiveMember> &Members,
                         StringRef FileName, bool FlattenArchive) {
    Expected<NewArchiveMember> NMOrErr =
            NewArchiveMember::getFile(FileName, Deterministic);
    if (isError(NMOrErr.takeError(), FileName)) {
        return false;
    }
    StringSaver Saver(Alloc);
    // For regular archives, use the basename of the object path for the member
    // name. For thin archives, use the full relative paths so the file resolves
    // correctly.
    if (!Thin) {
        NMOrErr->MemberName = llvm::sys::path::filename(NMOrErr->MemberName);
    } else {
        if (sys::path::is_absolute(FileName))
            NMOrErr->MemberName = Saver.save(sys::path::convert_to_slash(FileName));
        else {
            Expected<std::string> PathOrErr =
                    computeArchiveRelativePath(ArchiveName, FileName);
            NMOrErr->MemberName = Saver.save(
                    PathOrErr ? *PathOrErr : llvm::sys::path::convert_to_slash(FileName));
        }
    }

    if (FlattenArchive &&
        identify_magic(NMOrErr->Buf->getBuffer()) == file_magic::archive) {
        object::Archive *Lib = readLibrary(FileName);
        if (Lib == nullptr) return false;
        // When creating thin archives, only flatten if the member is also thin.
        if (!Thin || Lib->isThin()) {
            Error Err = Error::success();
            // Only Thin archives are recursively flattened.
            for (auto &Child: Lib->children(Err))
                addChildMember(Members, Child, /*FlattenArchive=*/Thin);
            if (isError(std::move(Err))) {
                return false;
            }
            return true;
        }
    }
    Members.push_back(std::move(*NMOrErr));
    return true;
}

bool Archiver::computeInsertAction(InsertAction &Action, ArchiveOperation Operation,
                                   const object::Archive::Child &Member,
                                   StringRef Name,
                                   std::vector<StringRef>::iterator &Pos,
                                   StringMap<int> &MemberCount) {

    if (Operation == QuickAppend || Members.empty())
        return IA_AddOldMember;
    auto MI = find_if(
            Members, [Name, this](StringRef Path) { return comparePaths(Name, Path); });

    if (MI == Members.end())
        return IA_AddOldMember;

    Pos = MI;

    if (Operation == Delete) {
        if (CountParam && ++MemberCount[Name] != CountParam)
            return IA_AddOldMember;
        return IA_Delete;
    }

    if (Operation == Move)
        return IA_MoveOldMember;

    if (Operation == ReplaceOrInsert) {
        if (!OnlyUpdate) {
            if (RelPos.empty())
                return IA_AddNewMember;
            return IA_MoveNewMember;
        }

        // We could try to optimize this to a fstat, but it is not a common
        // operation.
        llvm::sys::fs::file_status Status;
        if (isError(sys::fs::status(*MI, Status), *MI)) {
            return false;
        }
        auto ModTimeOrErr = Member.getLastModified();
        if (isError(ModTimeOrErr.takeError())) {
            return false;
        }
        if (Status.getLastModificationTime() < ModTimeOrErr.get()) {
            if (RelPos.empty())
                return IA_AddOldMember;
            return IA_MoveOldMember;
        }

        if (RelPos.empty())
            return IA_AddNewMember;
        return IA_MoveNewMember;
    }
    llvm_unreachable("No such operation");
}

// We have to walk this twice and computing it is not trivial, so creating an
// explicit std::vector is actually fairly efficient.
bool Archiver::computeNewArchiveMembers(ArchiveOperation Operation,
                                        object::Archive *OldArchive,
                                        std::vector<NewArchiveMember> &Ret) {
    std::vector<NewArchiveMember> Moved;
    int InsertPos = -1;
    if (OldArchive) {
        Error Err = Error::success();
        StringMap<int> MemberCount;
        for (auto &Child: OldArchive->children(Err)) {
            int Pos = Ret.size();
            Expected<StringRef> NameOrErr = Child.getName();
            if (isError(NameOrErr.takeError())) {
                return false;
            }
            std::string Name = std::string(NameOrErr.get());
            if (comparePaths(Name, RelPos)) {
                assert(AddAfter || AddBefore);
                if (AddBefore)
                    InsertPos = Pos;
                else
                    InsertPos = Pos + 1;
            }

            std::vector<StringRef>::iterator MemberI = Members.end();
            InsertAction Action;
            if (!computeInsertAction(Action, Operation, Child, Name, MemberI, MemberCount)) {
                return false;
            }

            switch (Action) {
                case IA_AddOldMember:
                    addChildMember(Ret, Child, /*FlattenArchive=*/Thin);
                    break;
                case IA_AddNewMember:
                    addMember(Ret, *MemberI);
                    break;
                case IA_Delete:
                    break;
                case IA_MoveOldMember:
                    addChildMember(Moved, Child, /*FlattenArchive=*/Thin);
                    break;
                case IA_MoveNewMember:
                    addMember(Moved, *MemberI);
                    break;
            }
            // When processing elements with the count param, we need to preserve the
            // full members list when iterating over all archive members. For
            // instance, "llvm-ar dN 2 archive.a member.o" should delete the second
            // file named member.o it sees; we are not done with member.o the first
            // time we see it in the archive.
            if (MemberI != Members.end() && !CountParam)
                Members.erase(MemberI);
        }
        if (isError(std::move(Err))) {
            return false;
        }
    }

    if (Operation == Delete)
        return true;

    if (!RelPos.empty() && InsertPos == -1) {
        return fail("insertion point not found");
    }

    if (RelPos.empty())
        InsertPos = Ret.size();

    assert(unsigned(InsertPos) <= Ret.size());
    int Pos = InsertPos;
    for (auto &M: Moved) {
        Ret.insert(Ret.begin() + Pos, std::move(M));
        ++Pos;
    }

    if (AddLibrary) {
        assert(Operation == QuickAppend);
        for (auto &Member: Members)
            addMember(Ret, Member, /*FlattenArchive=*/true);
        return true;
    }

    std::vector<NewArchiveMember> NewMembers;
    for (auto &Member: Members)
        addMember(NewMembers, Member, /*FlattenArchive=*/Thin);
    Ret.reserve(Ret.size() + NewMembers.size());
    std::move(NewMembers.begin(), NewMembers.end(),
              std::inserter(Ret, std::next(Ret.begin(), InsertPos)));

    return true;
}

object::Archive::Kind Archiver::getDefaultForHost() {
    return Triple(sys::getProcessTriple()).isOSDarwin()
           ? object::Archive::K_DARWIN
           : object::Archive::K_GNU;
}

object::Archive::Kind Archiver::getKindFromMember(const NewArchiveMember &Member) {
    auto MemBufferRef = Member.Buf->getMemBufferRef();
    Expected<std::unique_ptr<object::ObjectFile>> OptionalObject =
            object::ObjectFile::createObjectFile(MemBufferRef);

    if (OptionalObject)
        return isa<object::MachOObjectFile>(**OptionalObject)
               ? object::Archive::K_DARWIN
               : object::Archive::K_GNU;

    // squelch the error in case we had a non-object file
    consumeError(OptionalObject.takeError());

    // If we're adding a bitcode file to the archive, detect the Archive kind
    // based on the target triple.
    LLVMContext Context;
    if (identify_magic(MemBufferRef.getBuffer()) == file_magic::bitcode) {
        if (auto ObjOrErr = object::SymbolicFile::createSymbolicFile(
                MemBufferRef, file_magic::bitcode, &Context)) {
            auto &IRObject = cast<object::IRObjectFile>(**ObjOrErr);
            return Triple(IRObject.getTargetTriple()).isOSDarwin()
                   ? object::Archive::K_DARWIN
                   : object::Archive::K_GNU;
        } else {
            // Squelch the error in case this was not a SymbolicFile.
            consumeError(ObjOrErr.takeError());
        }
    }

    return getDefaultForHost();
}

bool Archiver::performReadOperation(ArchiveOperation Operation, object::Archive *OldArchive) {
    if (Operation == Extract && OldArchive->isThin())
        return fail("extracting from a thin archive is not supported");

    bool Filter = !Members.empty();
    StringMap<int> MemberCount;
    {
        Error Err = Error::success();
        for (auto &C: OldArchive->children(Err)) {
            Expected<StringRef> NameOrErr = C.getName();
            if (isError(NameOrErr.takeError())) {
                return false;
            }
            StringRef Name = NameOrErr.get();

            if (Filter) {
                auto I = find_if(Members, [Name, this](StringRef Path) {
                    return comparePaths(Name, Path);
                });
                if (I == Members.end())
                    continue;
                if (CountParam && ++MemberCount[Name] != CountParam)
                    continue;
                Members.erase(I);
            }

            switch (Operation) {
                default:
                    llvm_unreachable("Not a read operation");
                    break;
                case Extract:
                    if (!doExtract(Name, C)) {
                        return false;
                    }
                    break;
            }
        }
        if (isError(std::move(Err))) {
            return false;
        }
    }

    if (Members.empty())
        return true;
    for (StringRef Name: Members)
        fail("'" + Name + "' was not found");
    return false;
}

bool Archiver::performWriteOperation(ArchiveOperation Operation,
                                     object::Archive *OldArchive,
                                     std::unique_ptr<MemoryBuffer> OldArchiveBuf,
                                     std::vector<NewArchiveMember> *NewMembersP) {
    std::vector<NewArchiveMember> NewMembers;
    if (!NewMembersP)
        if (!computeNewArchiveMembers(Operation, OldArchive, NewMembers)) {
            return false;
        }

    object::Archive::Kind Kind;
    switch (FormatType) {
        case Default:
            if (Thin)
                Kind = object::Archive::K_GNU;
            else if (OldArchive)
                Kind = OldArchive->kind();
            else if (NewMembersP)
                Kind = !NewMembersP->empty() ? getKindFromMember(NewMembersP->front())
                                             : getDefaultForHost();
            else
                Kind = !NewMembers.empty() ? getKindFromMember(NewMembers.front())
                                           : getDefaultForHost();
            break;
        case GNU:
            Kind = object::Archive::K_GNU;
            break;
        case BSD:
            if (Thin)
                return fail("only the gnu format has a thin mode");
            Kind = object::Archive::K_BSD;
            break;
        case DARWIN:
            if (Thin)
                return fail("only the gnu format has a thin mode");
            Kind = object::Archive::K_DARWIN;
            break;
        case Unknown:
            llvm_unreachable("Unknown format");
    }

    Error E = writeArchive(ArchiveName, NewMembersP ? *NewMembersP : NewMembers, Symtab,
                           Kind, Deterministic, Thin, std::move(OldArchiveBuf));
    if (isError(std::move(E), ArchiveName)) {
        return false;
    }

    return true;
}
