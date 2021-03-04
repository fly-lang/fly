//===--------------------------------------------------------------------------------------------------------------===//
// src/Compiler/CompilerInvocation.cpp - Compiler Invocation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Frontend/CompilerInvocation.h"

using namespace fly;

CompilerInvocation::CompilerInvocation(IntrusiveRefCntPtr<DiagnosticsEngine> &&diags,
                                       IntrusiveRefCntPtr<TargetInfo> &&target,
                                       std::unique_ptr<FrontendOptions> &&frontendOptions,
                                       std::unique_ptr<CodeGenOptions> &&codeGenOptions) :
        diags(std::move(diags)), target(std::move(target)),
        frontendOpts(std::move(frontendOptions)), codeGenOpts(std::move(codeGenOptions)) {

    /// Create file manager.
    createFileManager();

    /// Create source manager.
    createSourceManager();
}

// File Manager

void CompilerInvocation::createFileManager(IntrusiveRefCntPtr<llvm::vfs::FileSystem> vfs) {
    if (!vfs)
        vfs = fileMgr ? &fileMgr->getVirtualFileSystem() : createVFS();
    assert(vfs && "FileManager has no VFS?");
    FileSystemOptions FileMgrOpts;
    fileMgr = new FileManager(FileMgrOpts, std::move(vfs));
}

// Source Manager

void CompilerInvocation::createSourceManager() {
    sourceMgr = new SourceManager(getDiagnostics(), getFileManager());
}

IntrusiveRefCntPtr<llvm::vfs::FileSystem>
CompilerInvocation::createVFS() {
    return llvm::vfs::getRealFileSystem();
}

/// Get the current diagnostics engine.
DiagnosticsEngine &CompilerInvocation::getDiagnostics() const {
    assert(diags && "Compiler instance has no diagnostics!");
    return *diags;
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
    assert(frontendOpts && "Compiler invocation has no Frontend options!");
    return *frontendOpts;
}

TargetInfo &CompilerInvocation::getTargetInfo() const {
    assert(target && "Compiler invocation has no target info!");
    return *target;
}

CodeGenOptions &CompilerInvocation::getCodeGenOptions() const {
    assert(codeGenOpts && "Compiler invocation has no CodeGen options");
    return *codeGenOpts;
}

