//===----------------------------------------------------------------------===//
// Compiler/CompilerInvocation.cpp - Compiler Invocation
//
// Part of the Fly Project, under the Apache License v2.0
// See https://flylang.org/LICENSE.txt for license information.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//

#include "Compiler/CompilerInvocation.h"
#include "Driver/Options.h"
#include <llvm/Option/OptTable.h>
#include <llvm/Option/ArgList.h>

using namespace fly;
using namespace fly::driver;


CompilerInvocation::CompilerInvocation(DiagnosticsEngine &diagnostics,
                                       const std::string& path, ArrayRef<const char *> argList) :
        diagnostics(diagnostics) {

    createInputOptions(path, argList.slice(1));

    /// Create file manager.
    createFileManager();

    /// Create source manager.
    createSourceManager();
}

// Input Options

void CompilerInvocation::createInputOptions(const std::string &path, llvm::ArrayRef<const char *> argList) {
    // TODO create CompilerInputFile instances for each input files
    // TODO create CompilerInputOptions instance
}

// File Manager

void CompilerInvocation::createFileManager(IntrusiveRefCntPtr<llvm::vfs::FileSystem> vfs) {
    if (!vfs)
        vfs = fileMgr ? &fileMgr->getVirtualFileSystem()
                      : createVFSFromCompilerInvocation(diagnostics);
    assert(vfs && "FileManager has no VFS?");
    FileSystemOptions FileMgrOpts;
    fileMgr = new FileManager(FileMgrOpts, std::move(vfs));
}

// Source Manager

void CompilerInvocation::createSourceManager() {
    sourceMgr = new SourceManager(getDiagnostics(), getFileManager());
}

IntrusiveRefCntPtr<llvm::vfs::FileSystem>
CompilerInvocation::createVFSFromCompilerInvocation(DiagnosticsEngine &diagnostics) {
    IntrusiveRefCntPtr<llvm::vfs::FileSystem> Result = llvm::vfs::getRealFileSystem();
    std::vector<std::string> Files;
    // earlier vfs files are on the bottom
    for (const auto &inputFile : inputFiles) {
        llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> Buffer =
                Result->getBufferForFile(inputFile.getFile());
        if (!Buffer) {
            diagnostics.Report(diag::err_missing_vfs_overlay_file) << inputFile.getFile();
            continue;
        }

        IntrusiveRefCntPtr<llvm::vfs::FileSystem> FS = llvm::vfs::getVFSFromYAML(
                std::move(Buffer.get()), /*DiagHandler*/ nullptr, inputFile.getFile(),
                /*DiagContext*/ nullptr, Result);
        if (!FS) {
            diagnostics.Report(diag::err_invalid_vfs_overlay) << inputFile.getFile();
            continue;
        }

        Result = FS;
    }
    return Result;
}
