//===--------------------------------------------------------------------------------------------------------------===//
// src/Frontend/Frontend.cpp - Main Compiler Frontend
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
#include "Frontend/Frontend.h"
#include "CodeGen/CodeGen.h"

using namespace fly;

Frontend::Frontend(CompilerInstance &CI) : CI(CI), Diags(CI.getDiagnostics()), Context(new ASTContext) {

    // Create Compiler Instance for each input file
    for (const auto &InputFile : CI.getFrontendOptions().getInputFiles()) {

        // Print file name and create instance for file compilation
        llvm::outs() << llvm::sys::path::filename(InputFile.getFile()) << "\n";
        Actions.push_back(new FrontendAction(CI, InputFile, Context));
    }
    llvm::outs().flush();
}

Frontend::~Frontend() {
    delete Context;
}

bool Frontend::Execute() const {
    bool Success = true;

    for (auto Action : Actions) {
        Diags.getClient()->BeginSourceFile();

        // Create ASTNode instance
        Success &= Action->BuildAST();

        if (!Success) {
            break;
        }

        Diags.getClient()->EndSourceFile();
    }

    if (Success) {
        
        // Generate Backend Code
        CodeGen CG(Diags, CI.getCodeGenOptions(), CI.getTargetOptions(), *Context,
                   CI.getFrontendOptions().getBackendAction());
        return CG.Execute();
    }

    return Success;
}

const std::vector<FrontendAction *> &Frontend::getActions() const {
    return Actions;
}
