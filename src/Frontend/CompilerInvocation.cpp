//===----------------------------------------------------------------------===//
// src/Compiler/CompilerInvocation.cpp - Compiler Invocation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//

#include "Frontend/CompilerInvocation.h"

using namespace fly;

CompilerInvocation::CompilerInvocation(IntrusiveRefCntPtr<DiagnosticsEngine> diagnostics,
                                       std::shared_ptr<FrontendOptions> frontendOptions,
                                       IntrusiveRefCntPtr<TargetInfo> target) :
        diagnostics(std::move(diagnostics)), frontendOptions(std::move(frontendOptions)), target(std::move(target)) {

    /// Create file manager.
    createFileManager();

    /// Create source manager.
    createSourceManager();
}

// File Manager

void CompilerInvocation::createFileManager(IntrusiveRefCntPtr<llvm::vfs::FileSystem> vfs) {
    if (!vfs)
        vfs = fileMgr ? &fileMgr->getVirtualFileSystem() : createVFSFromCompilerInvocation();
    assert(vfs && "FileManager has no VFS?");
    FileSystemOptions FileMgrOpts;
    fileMgr = new FileManager(FileMgrOpts, std::move(vfs));
}

// Source Manager

void CompilerInvocation::createSourceManager() {
    sourceMgr = new SourceManager(getDiagnostics(), getFileManager());
}

IntrusiveRefCntPtr<llvm::vfs::FileSystem>
CompilerInvocation::createVFSFromCompilerInvocation() {
    IntrusiveRefCntPtr<llvm::vfs::FileSystem> Result = llvm::vfs::getRealFileSystem();
    std::vector<std::string> Files;
    // earlier vfs files are on the bottom
    for (const auto &inputFile : frontendOptions->getInputFiles()) {
        llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> Buffer =
                Result->getBufferForFile(inputFile.getFile());
        if (!Buffer) {
            getDiagnostics().Report(diag::err_missing_vfs_overlay_file) << inputFile.getFile();
            continue;
        }

        IntrusiveRefCntPtr<llvm::vfs::FileSystem> FS = llvm::vfs::getVFSFromYAML(
                std::move(Buffer.get()), /*DiagHandler*/ nullptr, inputFile.getFile(),
                /*DiagContext*/ nullptr, Result);
        if (!FS) {
            getDiagnostics().Report(diag::err_invalid_vfs_overlay) << inputFile.getFile();
            continue;
        }

        Result = FS;
    }
    return Result;
}

/// Get the current diagnostics engine.
DiagnosticsEngine &CompilerInvocation::getDiagnostics() const {
    assert(diagnostics && "Compiler instance has no diagnostics!");
    return *diagnostics;
}

FileManager &CompilerInvocation::getFileManager() const {
    assert(fileMgr && "Compiler invocation has no file manager!");
    return *fileMgr;
}

SourceManager &CompilerInvocation::getSourceManager() const {
    assert(sourceMgr && "Compiler invocation has no file manager!");
    return *sourceMgr;
}

FrontendOptions &CompilerInvocation::getFrontendOptions() const {
    assert(frontendOptions && "Compiler invocation has no frontend options!");
    return *frontendOptions;
}

TargetInfo &CompilerInvocation::getTargetInfo() const {
    assert(target && "Compiler invocation has no target info!");
    return *target;
}
