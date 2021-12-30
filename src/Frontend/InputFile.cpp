//
// Created by marco on 5/22/21.
//

#include "Frontend/InputFile.h"
#include "Basic/Debug.h"

using namespace fly;

InputFile::InputFile(std::string FileName) : FileName(FileName) {

}

bool InputFile::Load(llvm::StringRef Source, SourceManager &SourceMgr) {
    FLY_DEBUG_MESSAGE("InputFile", "Load", "Source=" + Source);
    // Set Source Manager file id
    std::unique_ptr<llvm::MemoryBuffer> Buf = llvm::MemoryBuffer::getMemBuffer(Source, FileName);

    Buffer = Buf.get();
    FID = SourceMgr.createFileID(std::move(Buf));
//    SourceMgr.setMainFileID(FID);
    return true;
}

bool InputFile::Load(SourceManager &SourceMgr, DiagnosticsEngine &Diags) {
    FLY_DEBUG_MESSAGE("InputFile", "Load", "File=" + FileName);
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
