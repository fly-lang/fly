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

void FrontendOptions::addInputFile(std::string &&input) {
    inputFiles.emplace_back(InputFile(input));
}

const llvm::SmallVector<InputFile, 0> &FrontendOptions::getInputFiles() const {
    return inputFiles;
}

void FrontendOptions::setOutputFile(const char * output) {
    FrontendOptions::outputFile.setFile(output);
}

const OutputFile &FrontendOptions::getOutputFile() const {
    return outputFile;
}

bool FrontendOptions::isVerbose() const {
    return verbose;
}

void FrontendOptions::setVerbose() {
    FrontendOptions::verbose = true;
}

bool FrontendOptions::isSkipParse() const {
    return skipParse;
}

void FrontendOptions::setSkipParse() {
    skipParse = true;
}
