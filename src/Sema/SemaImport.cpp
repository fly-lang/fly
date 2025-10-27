//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaImport.cpp - The Sema Import implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaImport.h"

using namespace fly;

SemaImport::SemaImport(ASTImport &AST) : AST(AST) {
}