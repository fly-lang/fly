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
#include "CodeGen/CharUnits.h"
#include "CodeGen/CodeGenHeader.h"
#include "Sema/SemaNameSpace.h"
#include "AST/ASTModule.h"
#include "AST/ASTNameSpace.h"
#include "Frontend/FrontendOptions.h"
#include "Basic/FileManager.h"
#include "Basic/TargetInfo.h"
#include "Basic/Debug.h"

#include <Sema/SymbolTable.h>
#include <llvm/IR/LLVMContext.h>

using namespace fly;

CodeGen::CodeGen(DiagnosticsEngine &Diags, CodeGenOptions &CodeGenOpts,
                 const std::shared_ptr<TargetOptions> &TargetOpts,
                 BackendActionKind BackendAction, bool ShowTimers) :
        Diags(Diags), CodeGenOpts(CodeGenOpts), TargetOpts(*TargetOpts),
        Target(CreateTargetInfo(Diags, TargetOpts)), ActionKind(BackendAction),
        ShowTimers(ShowTimers) {
}

std::string CodeGen::getOutputFileName(llvm::StringRef BaseInput) {
    StringRef FileName = llvm::sys::path::filename(BaseInput);
    std::string Name = FileName.str();//.substr(0,FileName.size()-4/* sizeof('.fly') = 4 */).str();
    switch (ActionKind) {
        case Backend_EmitNothing:
            FLY_DEBUG_START_MSG("CodeGen", "getOutputFileName","return ''");
            return "";
        case Backend_EmitLL:
            FLY_DEBUG_START_MSG("CodeGen", "getOutputFileName","return " << Name + ".ll");
            return Name + ".ll";
        case Backend_EmitBC:
            FLY_DEBUG_START_MSG("CodeGen", "getOutputFileName","return " << Name + ".bc");
            return Name + ".bc";
        case Backend_EmitAssembly:
            FLY_DEBUG_START_MSG("CodeGen", "getOutputFileName","return " << Name + ".s");
            return Name + ".s";
        case Backend_EmitObj:
            FLY_DEBUG_START_MSG("CodeGen", "getOutputFileName","return " << Name + ".o");
            return Name + ".o";
    }

    llvm_unreachable("Invalid backend action!");
}

std::string str(llvm::Module *M) {
    std::string str;
    llvm::raw_string_ostream rso(str);
    M->print(rso, nullptr);
    return str;
}

void CodeGen::Emit(llvm::Module *M, llvm::StringRef OutName) {
    FLY_DEBUG_START_MSG("CodeGen", "Emit",
                      "Module.Name=" << M->getName() << "\nModule.Output=" << str(M));

    std::string OutputFileName = OutName.empty() ? getOutputFileName(M->getName()) : OutName.str();
    // TODO link to libraries
//    Link = new llvm::Linker(*OutModule);
//    Linker::linkModules(*OutModule, std::move(M));

    // Skip CodeGenModule instance creation
    if (ActionKind == Backend_EmitNothing) {
        return;
    }

    std::error_code ErrCode;
    std::unique_ptr<llvm::raw_fd_ostream> OS =
            std::make_unique<llvm::raw_fd_ostream>(OutName, ErrCode, llvm::sys::fs::F_None);

    // Include Bitcode in module
    EmbedBitcode(M, CodeGenOpts, llvm::MemoryBufferRef());

    EmitBackendOutput(Diags, CodeGenOpts, TargetOpts, ShowTimers, Target->getDataLayout(),M, ActionKind, std::move(OS));
}

TargetInfo* CodeGen::CreateTargetInfo(DiagnosticsEngine &Diags,
                                      const std::shared_ptr<TargetOptions> &TargetOpts) {
    return TargetInfo::CreateTargetInfo(Diags, TargetOpts);
}

TargetInfo &CodeGen::getTargetInfo() const {
    assert(Target && "Compiler invocation has no target info!");
    return *Target;
}

llvm::LLVMContext &CodeGen::getLLVMCtx() {
    return LLVMCtx;
}

std::vector<llvm::Module *> CodeGen::GenerateModules(llvm::SmallVector<SemaModule *, 8> &SemaModules) {
    FLY_DEBUG_START("CodeGen", "GenerateModules");
    std::vector<llvm::Module *> Modules;
    for (auto &SemaModule : SemaModules) {
        Diags.getClient()->BeginSourceFile();
        llvm::Module *M = GenerateModule(SemaModule);
        Modules.push_back(M);
        Diags.getClient()->EndSourceFile();
    }
    return Modules;
}

llvm::Module *CodeGen::GenerateModule(SemaModule *Module) {
    FLY_DEBUG_START("CodeGen", "GenerateModule");
    CodeGenModule *CGM = new CodeGenModule(Diags, Module, LLVMCtx, *Target, CodeGenOpts);
    return CGM->getModule();
}

