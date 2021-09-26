//
// Created by marco on 5/22/21.
//

#include "Frontend/InputFile.h"

using namespace fly;

InputFile::InputFile(llvm::StringRef File) : File(File) {

}

bool InputFile::Load(llvm::StringRef Source, SourceManager &SourceMgr) {
    // Set Source Manager file id
    std::unique_ptr<llvm::MemoryBuffer> Buf = llvm::MemoryBuffer::getMemBuffer(Source, File);

    Buffer = Buf.get();
    FID = SourceMgr.createFileID(std::move(Buf));
//    SourceMgr.setMainFileID(FID);
    return true;
}

bool InputFile::Load(SourceManager &SourceMgr, DiagnosticsEngine &Diags) {
    llvm::ErrorOr <std::unique_ptr<llvm::MemoryBuffer>> FileBuf = llvm::MemoryBuffer::getFileOrSTDIN(getFile());

    // Check file error
    if (FileBuf.getError()) {
        Diags.Report(diag::err_cannot_open_file) << getFile() << FileBuf.getError().message();
        return false;
    }

    // Create FileId for Lexer
    std::unique_ptr<llvm::MemoryBuffer> &Buf = FileBuf.get();
    Buffer = Buf.get();
    FID = SourceMgr.createFileID(std::move(Buf));
//    SourceMgr.setMainFileID(FID); // TODO set for main file
    return true;
}
