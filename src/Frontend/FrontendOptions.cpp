//===----------------------------------------------------------------------===//
// src/Compiler/InputOptions.cpp - Compiler Input Options
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//

#include "Frontend/FrontendOptions.h"

using namespace fly;

FrontendOptions::FrontendOptions() : outputFile(OutputFile("file")) {

}

FrontendOptions::FrontendOptions(const llvm::SmallVector<InputFile, 0> &inputFiles, const OutputFile &outputFile)
        : inputFiles(inputFiles), outputFile(outputFile) {}

FrontendOptions::FrontendOptions(const llvm::SmallVector<InputFile, 0> &inputFiles)
        : outputFile(OutputFile("file")) {

}

void FrontendOptions::addInputFile(InputFile inputFile) {
    inputFiles.push_back(std::move(inputFile));
}

const llvm::SmallVector<InputFile, 0> &FrontendOptions::getInputFiles() const {
    return inputFiles;
}

const OutputFile &FrontendOptions::getOutputFile() const {
    return outputFile;
}
