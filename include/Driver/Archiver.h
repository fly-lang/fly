//===--------------------------------------------------------------------------------------------------------------===//
// include/Driver/Archiver.h - Create and Extract Archive
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Basic/FileManager.h"
#include "Basic/Diagnostic.h"

#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/Triple.h"
#include "llvm/BinaryFormat/Magic.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Object/Archive.h"
#include "llvm/Object/ArchiveWriter.h"
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
#include "llvm/Support/LineIterator.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/StringSaver.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ToolDrivers/llvm-dlltool/DlltoolDriver.h"
#include "llvm/ToolDrivers/llvm-lib/LibDriver.h"

#if !defined(_MSC_VER) && !defined(__MINGW32__)
#include <unistd.h>
#else
#include <io.h>
#endif

namespace fly {

    using namespace llvm;

    enum Format {
        Default, GNU, BSD, DARWIN, Unknown
    };

    enum InsertAction {
        IA_AddOldMember,
        IA_AddNewMember,
        IA_Delete,
        IA_MoveOldMember,
        IA_MoveNewMember
    };

    // This enumeration delineates the kinds of operations on an archive
    // that are permitted.
    enum ArchiveOperation {
        Delete,          ///< Delete the specified members
        Move,            ///< Move members to end or as given by {a,b,i} modifiers
        QuickAppend,     ///< Quickly append to end of archive
        ReplaceOrInsert, ///< Replace or Insert members
        Extract,         ///< Extract files back to file system
        CreateSymTab     ///< Create a symbol table in an existing archive
    };

    /**
     *
     *  Archiver
     *
     *  USAGE: llvm-ar [options] [-]<operation>[modifiers] [relpos] [count] <archive> [files]
     *
     *  OPERATIONS:
     *    d - delete [files] from the archive
     *    m - move [files] in the archive
     *    q - quick append [files] to the archive
     *    r - replace or insert [files] into the archive
     *    s - act as ranlib
     *    x - extract [files] from the archive
     *
     *   expected [relpos] for 'a', 'b', or 'i' modifier
     *   expected [count] for 'N' modifier
     *   value for [count] must be numeric
    */
    class Archiver {

        DiagnosticsEngine &Diag;

        Format FormatType = Default;

        // Modifiers to follow operation to vary behavior
        bool AddAfter = false;             ///< 'a' - put [files] after [relpos]
        bool AddBefore = false;            ///< 'b' - put [files] before [relpos] (same as [i])
        bool Create = false;               ///< 'c' - do not warn if archive had to be created
        bool OriginalDates = false;        ///< 'o' - preserve original dates
        bool CompareFullPath = false;      ///< 'P' - use full names when matching (implied for thin archives)
        bool OnlyUpdate = false;           ///< 'u' - update only [files] newer than archive contents
        bool Symtab = true;                ///< 's' - create an archive index
        bool Deterministic = true;         ///< 'D' - use zero for timestamps and uids/gids (default)
                                           ///< 'U' - use actual timestamps and uids/gids
        bool Thin = false;                 ///< 'T' - create a thin archive // Thin archives store path names, so P should be forced.
        bool AddLibrary = false;           ///< 'L' - add archive's contents

        // Relative Positional Argument (for insert/move). This variable holds
        // the name of the archive member to which the 'a', 'b' or 'i' modifier
        // refers. Only one of 'a', 'b' or 'i' can be specified so we only need
        // one variable.
        std::string RelPos;

        // Count parameter for 'N' modifier. This variable specifies which file should
        // match for extract/delete operations when there are multiple matches. This is
        // 1-indexed. A value of 0 is invalid, and implies 'N' is not used.
        int CountParam = 0;

        // This variable holds the name of the archive file as given on the
        // command line.
        std::string ArchiveName;

        std::vector <std::unique_ptr<MemoryBuffer>> ArchiveBuffers;
        std::vector <std::unique_ptr<object::Archive>> Archives;

        // This variable holds the list of member files to proecess, as given
        // on the command line.
        std::vector <llvm::StringRef> Members;

        // Static buffer to hold StringRefs.
        BumpPtrAllocator Alloc;

    public:
        Archiver(DiagnosticsEngine &Diag, const std::string &ArchiveName);
        bool CreateLib(const llvm::SmallVector<std::string, 4> &Files);
        std::vector<std::string> ExtractFiles(FileManager &FileMgr);

    private:
        bool fail(Twine Error);
        bool isError(std::error_code EC, Twine Context = "");
        bool isError(Error E, Twine Context = "");
        object::Archive *readLibrary(const Twine &Library);
        std::string normalizePath(StringRef Path);
        bool comparePaths(StringRef Path1, StringRef Path2);
        bool doExtract(StringRef Name, const object::Archive::Child &C);
        bool addChildMember(std::vector<NewArchiveMember> &Members, const object::Archive::Child &M,
                            bool FlattenArchive = false);
        bool addMember(std::vector<NewArchiveMember> &Members,
                                 StringRef FileName, bool FlattenArchive = false);
        bool computeInsertAction(InsertAction &Action, ArchiveOperation Operation, const object::Archive::Child &Member,
                                         StringRef Name, std::vector<StringRef>::iterator &Pos,
                                         StringMap<int> &MemberCount);
        bool computeNewArchiveMembers(ArchiveOperation Operation, object::Archive *OldArchive,
                                      std::vector<NewArchiveMember> &Ret);
        object::Archive::Kind getDefaultForHost();
        object::Archive::Kind getKindFromMember(const NewArchiveMember &Member);
        bool performReadOperation(ArchiveOperation Operation, object::Archive *OldArchive);
        bool performWriteOperation(ArchiveOperation Operation, object::Archive *OldArchive,
                                   std::unique_ptr<MemoryBuffer> OldArchiveBuf,
                                   std::vector<NewArchiveMember> *NewMembersP);
        bool createSymbolTable(object::Archive *OldArchive);
        bool performOperation(ArchiveOperation Operation, object::Archive *OldArchive,
                              std::unique_ptr<MemoryBuffer> OldArchiveBuf, std::vector<NewArchiveMember> *NewMembers);
        bool performOperation(ArchiveOperation Operation, std::vector<NewArchiveMember> *NewMembers);
    };
}