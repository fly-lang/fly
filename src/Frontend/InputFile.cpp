//===--------------------------------------------------------------------------------------------------------------===//
// src/Frontend/InputFile.cpp - Input File
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Frontend/InputFile.h"
#include "Basic/Debug.h"

using namespace fly;

FileExt TakeExt(std::string FileName) {
    if (FileName.substr(FileName.size() - 4, FileName.size()) == ".fly") {
        return FLY;
    } else if (FileName.substr(FileName.size() - 4, FileName.size()) == ".lib") {
        return LIB;
    } else if (FileName.substr(FileName.size() - 4, FileName.size()) == ".o") {
        return OBJ;
    }
    return UNKNOWN;
}

std::string TakeName(std::string FileName) {
    return FileName.substr(0, FileName.size()-4);
}

InputFile::InputFile(std::string FileName) : FileName(FileName), Name(TakeName(FileName)),
                                             Ext(TakeExt(FileName)) {

}

bool InputFile::Load(llvm::StringRef Source, SourceManager &SourceMgr) {
    FLY_DEBUG_MESSAGE("InputFile", "Load", "Source=" + Source);
    // Set Source Manager file id
    std::unique_ptr<llvm::MemoryBuffer> Buf = llvm::MemoryBuffer::getMemBuffer(Source, Name);

    Buffer = Buf.get();
    FID = SourceMgr.createFileID(std::move(Buf));
//    SourceMgr.setMainFileID(FID);
    return true;
}

bool InputFile::Load(SourceManager &SourceMgr, DiagnosticsEngine &Diags) {
    FLY_DEBUG_MESSAGE("InputFile", "Load", "File=" + Name);
    llvm::ErrorOr <std::unique_ptr<llvm::MemoryBuffer>> FileBuf = llvm::MemoryBuffer::getFileOrSTDIN(getFileName());

    // Check file error
    if (FileBuf.getError()) {
        Diags.Report(diag::err_cannot_open_file) << getFileName() << FileBuf.getError().message();
        return false;
    }

    // Create FileId for Lexer
    std::unique_ptr<llvm::MemoryBuffer> &Buf = FileBuf.get();
    Buffer = Buf.get();
    FID = SourceMgr.createFileID(std::move(Buf));
//    SourceMgr.setMainFileID(FID); // TODO set for main file
    return true;
}

bool InputFile::isEmpty() const {
    return !FileName.empty();
}

bool InputFile::isBuffer() const {
    return Buffer != nullptr;
}

const std::string &InputFile::getFileName() const {
    assert(isEmpty());
    return FileName;
}

const std::string &InputFile::getName() const {
    assert(isEmpty());
    return Name;
}

const FileExt &InputFile::getExt() const {
    assert(isEmpty());
    return Ext;
}

FileID InputFile::getFileID() const {
    assert(FID.isValid() && "Invalid FileID");
    return FID;
}

const llvm::MemoryBuffer *InputFile::getBuffer() const {
    assert(isBuffer());
    return Buffer;
}
