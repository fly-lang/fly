//===--------------------------------------------------------------------------------------------------------------===//
// src/Compiler/InputOptions.cpp - Compiler Input Options
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Frontend/FrontendOptions.h"
#include "Basic/Debug.h"

using namespace fly;

void FrontendOptions::addInputFile(const char * FileName) {
    FLY_DEBUG_MESSAGE("FrontendOptions", "addInputFile", "FileName=" << FileName);
    Inputs.emplace_back(FileName);
}

const llvm::SmallVector<std::string, 16> &FrontendOptions::getInputFiles() const {
    return Inputs;
}

void FrontendOptions::setOutputFile(llvm::StringRef FileName, bool isLib) {
    Output = std::string(FileName);
    OutputLib = isLib;
}

const std::string &FrontendOptions::getOutputFile() const {
    return Output;
}

bool FrontendOptions::isOutputLib() const {
    return OutputLib;
}
