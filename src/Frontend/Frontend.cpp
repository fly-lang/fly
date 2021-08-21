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

bool Frontend::Execute() {
    bool Success = true;
    unsigned NInputs = 0;

    // Generate Backend Code
    CodeGen CG(Diags, CI.getCodeGenOptions(), CI.getTargetOptions(),
               CI.getFrontendOptions().getBackendAction());

    // Create Compiler Instance for each input file
    for (auto InputFile : CI.getFrontendOptions().getInputFiles()) {
        // Print file name and create instance for file compilation
//        llvm::outs() << llvm::sys::path::filename(InputFile.getFile()) << "\n";
        InputFile.Load(CI.getSourceManager(), Diags);
        FrontendAction *Action = new FrontendAction(CI, Context, CG);
        if (!Diags.hasErrorOccurred()) {
            Diags.getClient()->BeginSourceFile();

            // Parse Action & add to Actions for next
            Success &= Action->Parse(InputFile);
            Actions.emplace_back(Action);

            Diags.getClient()->EndSourceFile();
            NInputs++;
        }
    }

    if (Success && NInputs > 0) {
        Context->Resolve();
        llvm::outs().flush();
        for (auto Action : Actions) {
            Action->Compile();
            Success &= Action->EmitOutput();
        }
    } else {
        Diags.Report(SourceLocation(), diag::note_no_input_process);
    }

    return Success;
}
