//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ClassDecl.cpp - Class implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ClassDecl.h"

using namespace fly;

const llvm::StringRef &ClassDecl::getName() const {
    return Name;
}