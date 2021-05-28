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

CodeGen::CodeGen(DiagnosticsEngine &Diags, CodeGenOptions &CodeGenOpts, const std::shared_ptr<TargetOptions> &TargetOpts,
                 ASTContext &Context, BackendAction ActionKind) :
        Diags(Diags), CodeGenOpts(CodeGenOpts), TargetOpts(*TargetOpts), Context(Context),
        Target(CreateTargetInfo(Diags, TargetOpts)), ActionKind(ActionKind) {
}



std::string CodeGen::getOutputFileName(BackendAction ActionKind, StringRef BaseInput) {
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

std::unique_ptr<llvm::Module>& CodeGen::getModule() {
    return Builder->Module;
}

bool CodeGen::Execute() {

    if (ActionKind == Backend_EmitNothing) {
        return true;
    }

    // Generate default module on first
    ASTNameSpace *Default = Context.getNameSpaces().lookup("default");
    if (Default)
        GenerateModule(Default);
    
    // After generate all other modules
    for(auto &EntryNS : Context.getNameSpaces()) {
        if (!EntryNS.getKey().equals("default")) {
            GenerateModule(EntryNS.getValue());
        }
    }
    return true;
}

TargetInfo* CodeGen::CreateTargetInfo(DiagnosticsEngine &Diags,
                                            const std::shared_ptr<TargetOptions> &TargetOpts) {
    return TargetInfo::CreateTargetInfo(Diags, TargetOpts);
}

TargetInfo &CodeGen::getTargetInfo() const {
    assert(Target && "Compiler invocation has no target info!");
    return *Target;
}

void CodeGen::GenerateModule(ASTNameSpace * NS) {
    for (auto &EntryNode : NS->getNodes()) {

        ASTNode *AST = EntryNode.getValue();

        std::string OutputFileName = getOutputFileName(ActionKind, AST->getFileName());
        std::error_code Code;
        std::unique_ptr<llvm::raw_fd_ostream> OS =
                std::make_unique<raw_fd_ostream>(OutputFileName, Code, llvm::sys::fs::F_None);

        Builder = std::make_unique<CodeGenModule>(Diags, *AST, *Target);
        Builder->GenAST();

        EmbedBitcode(Builder->Module.get(), CodeGenOpts, llvm::MemoryBufferRef());

        EmitBackendOutput(Diags, CodeGenOpts, TargetOpts, Target->getDataLayout(),
                          Builder->Module.get(), ActionKind, std::move(OS));
    }
}
