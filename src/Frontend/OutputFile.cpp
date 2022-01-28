//===--------------------------------------------------------------------------------------------------------------===//
// src/Frontend/OutputFile.cpp - Output File
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Frontend/OutputFile.h"

using namespace fly;

void OutputFile::setFile(const std::string &Name, bool isLib) {
    FileName = Name;
    Lib = isLib;
}

bool OutputFile::isLib() const {
    return Lib;
}

const std::string &OutputFile::getFile() const {
    return FileName;
}
