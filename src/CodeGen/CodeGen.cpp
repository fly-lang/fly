//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGen.cpp - Code Generator implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGen.h"
#include "Basic/FileManager.h"
#include <llvm/Target/TargetMachine.h>
#include <llvm/IR/LLVMContext.h>

using namespace fly;

CodeGen::CodeGen(DiagnosticsEngine &Diags, CodeGenOptions &CodeGenOpts, TargetOptions &TargetOpts,
                 ASTContext &Context, TargetInfo &Target, BackendAction ActionKind) :
        Diags(Diags), CodeGenOpts(CodeGenOpts), TargetOpts(TargetOpts), Context(Context),
        ModuleName(getModuleName(ActionKind, Context.getFileName())),
        Builder(new CodeGenModule(Diags, ModuleName)), Target(Target), ActionKind(ActionKind){

    Builder->Initialize(Target);
}

std::string CodeGen::getModuleName(BackendAction ActionKind, StringRef BaseInput) {
    StringRef FileName = llvm::sys::path::filename(BaseInput);
    std::string Name = FileName.substr(0,FileName.size()-4/* sizeof('.fly') = 4 */).str();
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

    llvm_unreachable("Invalid action!");
}

std::unique_ptr<llvm::raw_fd_ostream> CodeGen::getOutputStream(std::error_code &Code) {
    std::unique_ptr<llvm::raw_fd_ostream> OS;
    if (ActionKind == Backend_EmitNothing) {
        return nullptr;
    }
    return std::make_unique<raw_fd_ostream>(ModuleName, Code, llvm::sys::fs::F_None);
}

std::unique_ptr<llvm::Module>& CodeGen::getModule() {
    return Builder->Module;
}

bool CodeGen::execute() {

    std::error_code Code;
    std::unique_ptr<llvm::raw_fd_ostream>OS = getOutputStream(Code);

    EmbedBitcode(Builder->Module.get(), CodeGenOpts, llvm::MemoryBufferRef());

    EmitBackendOutput(Diags, CodeGenOpts, TargetOpts,Target.getDataLayout(),
                      Builder->Module.get(), ActionKind, std::move(OS));

    return true;
}
