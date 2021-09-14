//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGen.cpp - Code Generator implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "AST/ASTNode.h"
#include "AST/ASTNameSpace.h"
#include "Frontend/FrontendOptions.h"
#include "Basic/FileManager.h"
#include "Basic/TargetInfo.h"
#include <llvm/IR/LLVMContext.h>

using namespace fly;

CodeGen::CodeGen(DiagnosticsEngine &Diags, CodeGenOptions &CodeGenOpts,
                 const std::shared_ptr<TargetOptions> &TargetOpts,
                 BackendActionKind BackendAction, bool ShowTimers) :
        Diags(Diags), CodeGenOpts(CodeGenOpts), TargetOpts(*TargetOpts),
        Target(CreateTargetInfo(Diags, TargetOpts)), ActionKind(BackendAction), ShowTimers(ShowTimers) {
}

std::string CodeGen::getOutputFileName(BackendActionKind ActionKind, StringRef BaseInput) {
    StringRef FileName = llvm::sys::path::filename(BaseInput);
    std::string Name = FileName.str();//.substr(0,FileName.size()-4/* sizeof('.fly') = 4 */).str();
    switch (ActionKind) {
        case Backend_EmitNothing:
            return "";
        case Backend_EmitLL:
            return Name + ".ll";
        case Backend_EmitBC:
            return Name + ".bc";
        case Backend_EmitAssembly:
            return Name + ".as";
        case Backend_EmitObj:
            return Name + ".o";
    }

    llvm_unreachable("Invalid backend action!");
}

void CodeGen::Emit(CodeGenModule *CGM) {
    // Skip CodeGenModule instance creation
    if (ActionKind == Backend_EmitNothing) {
        return;
    }

    // After generate all other modules
    EmbedBitcode(CGM->Module, CodeGenOpts, llvm::MemoryBufferRef());

    std::string OutputFileName = getOutputFileName(ActionKind, CGM->getModule()->getName());
    std::error_code ErrCode;
    std::unique_ptr<llvm::raw_fd_ostream> OS =
            std::make_unique<raw_fd_ostream>(OutputFileName, ErrCode, llvm::sys::fs::F_None);
    EmitBackendOutput(Diags, CodeGenOpts, TargetOpts, ShowTimers, Target->getDataLayout(), CGM->Module,
                      ActionKind, std::move(OS));
}

TargetInfo* CodeGen::CreateTargetInfo(DiagnosticsEngine &Diags,
                                            const std::shared_ptr<TargetOptions> &TargetOpts) {
    return TargetInfo::CreateTargetInfo(Diags, TargetOpts);
}

TargetInfo &CodeGen::getTargetInfo() const {
    assert(Target && "Compiler invocation has no target info!");
    return *Target;
}

CodeGenModule *CodeGen::CreateModule(llvm::StringRef Name) {
    return new CodeGenModule(Diags, Name, LLVMCtx, *Target, CodeGenOpts);
}
