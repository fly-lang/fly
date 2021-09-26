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

void FrontendOptions::setOutputFile(llvm::StringRef FileName) {
    FrontendOptions::Output.setFile(FileName);
}

const OutputFile &FrontendOptions::getOutputFile() const {
    return Output;
}
