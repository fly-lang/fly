//===--------------------------------------------------------------------------------------------------------------===//
// src/Frontend/Frontend.cpp - Main Compiler Frontend
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
#include "Frontend/Frontend.h"

using namespace fly;

Frontend::Frontend(CompilerInstance &CI) : CI(CI), Diags(CI.getDiagnostics()) {

    // Create Compiler Instance for each input file
    for (const auto &InputFile : CI.getFrontendOptions().getInputFiles()) {

        // Print file name and create instance for file compilation
        llvm::outs() << llvm::sys::path::filename(InputFile.getFile()) << "\n";
        Actions.push_back(new FrontendAction(CI, InputFile));
    }
    llvm::outs().flush();
}

bool Frontend::execute() const {
    bool Res = true;
    for (auto &Action : Actions) {
        Res &= Action->Execute();
    }
    return Res;
}

const std::vector<FrontendAction *> &Frontend::getActions() const {
    return Actions;
}
