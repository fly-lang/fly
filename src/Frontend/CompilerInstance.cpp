//===--------------------------------------------------------------------------------------------------------------===//
// src/Compiler/CompilerInvocation.cpp - Compiler Invocation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Frontend/CompilerInstance.h"

using namespace fly;

CompilerInstance::CompilerInstance(IntrusiveRefCntPtr<DiagnosticsEngine> Diags,
                                   FileSystemOptions &&FileSystemOpts,
                                   std::shared_ptr<TargetOptions> &&TargetOptions,
                                   FrontendOptions *FrontendOptions,
                                   CodeGenOptions *CodeGenOptions) :
        Diags(Diags), FileSystemOpts(std::move(FileSystemOpts)),
        FrontendOpts(FrontendOptions), CodeGenOpts(CodeGenOptions),
        TargetOpts(std::move(TargetOptions)) {

    /// Create file manager.
    createFileManager();

    /// Create source manager.
    createSourceManager();
}

// File Manager

void CompilerInstance::createFileManager(IntrusiveRefCntPtr<llvm::vfs::FileSystem> VFS) {
    if (!VFS)
        VFS = FileMgr ? &FileMgr->getVirtualFileSystem() : llvm::vfs::getRealFileSystem();
    assert(VFS && "FileManager has no VFS?");

    // Set working dir
    if (!FileSystemOpts.WorkingDir.empty())
        if (VFS->setCurrentWorkingDirectory(FileSystemOpts.WorkingDir))
            Diags->Report(diag::err_drv_unable_to_set_working_directory) << FileSystemOpts.WorkingDir;

    FileMgr = new FileManager(FileSystemOpts, std::move(VFS));
}

// Source Manager

void CompilerInstance::createSourceManager() {
    SourceMgr = new SourceManager(getDiagnostics(), getFileManager());
}

/// Get the current diagnostics engine.
DiagnosticsEngine &CompilerInstance::getDiagnostics() const {
    assert(Diags && "Compiler instance has no diagnostics!");
    return *Diags;
}

FileManager &CompilerInstance::getFileManager() const {
    assert(FileMgr && "Compiler invocation has no file manager!");
    return *FileMgr;
}

SourceManager &CompilerInstance::getSourceManager() const {
    assert(SourceMgr && "Compiler invocation has no file manager!");
    return *SourceMgr;
}

FrontendOptions &CompilerInstance::getFrontendOptions() const {
    assert(FrontendOpts && "Compiler invocation has no Frontend options!");
    return *FrontendOpts;
}

CodeGenOptions &CompilerInstance::getCodeGenOptions() const {
    assert(CodeGenOpts && "Compiler invocation has no CodeGen options");
    return *CodeGenOpts;
}

const std::shared_ptr<fly::TargetOptions> &CompilerInstance::getTargetOptions() const {
    assert(TargetOpts && "Compiler invocation has no Target options");
    return TargetOpts;
}
