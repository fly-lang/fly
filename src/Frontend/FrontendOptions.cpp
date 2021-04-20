//===--------------------------------------------------------------------------------------------------------------===//
// src/Compiler/InputOptions.cpp - Compiler Input Options
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Frontend/FrontendOptions.h"

using namespace fly;

void FrontendOptions::addInputFile(llvm::StringRef FileName) {
    Inputs.emplace_back(InputFile(FileName));
}

const llvm::SmallVector<InputFile, 0> &FrontendOptions::getInputFiles() const {
    return Inputs;
}

void FrontendOptions::setOutputFile(const char * FileName) {
    FrontendOptions::Output.setFile(FileName);
}

const OutputFile &FrontendOptions::getOutputFile() const {
    return Output;
}

bool FrontendOptions::isVerbose() const {
    return Verbose;
}

void FrontendOptions::setVerbose() {
    FrontendOptions::Verbose = true;
}

BackendAction FrontendOptions::getBackendAction() {
    return Action;
}

void FrontendOptions::setBackendAction(BackendAction action) {
    Action = action;
}
