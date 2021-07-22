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

Frontend::Frontend(CompilerInstance &CI) : CI(CI), Diags(CI.getDiagnostics()), Context(new ASTContext(Diags)) {

}

Frontend::~Frontend() {
    delete Context;
}

bool Frontend::Execute() const {
    bool Success = true;
    unsigned NInputs = 0;

    // Create Compiler Instance for each input file
    for (auto InputFile : CI.getFrontendOptions().getInputFiles()) {
        // Print file name and create instance for file compilation
//        llvm::outs() << llvm::sys::path::filename(InputFile.getFile()) << "\n";
        FrontendAction *Action = new FrontendAction(CI, InputFile, Context);
        if (!Diags.hasErrorOccurred()) {
            Diags.getClient()->BeginSourceFile();

            // Create ASTNode instance
            Success &= Action->BuildASTNode();

            Diags.getClient()->EndSourceFile();
            NInputs++;
        }

    }

    if (Success && NInputs > 0) {
        Context->Finalize();

        llvm::outs().flush();
        
        // Generate Backend Code
        CodeGen CG(Diags, CI.getCodeGenOptions(), CI.getTargetOptions(), *Context,
                   CI.getFrontendOptions().getBackendAction());
        return CG.Execute();
    } else {
        Diags.Report(SourceLocation(), diag::note_no_input_process);
    }

    return Success;
}
