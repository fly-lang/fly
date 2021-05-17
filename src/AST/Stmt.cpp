//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/Stmt.cpp - Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/Stmt.h"

using namespace fly;

Stmt::Stmt(const SourceLocation &Loc) : Location(Loc) {}

const SourceLocation &Stmt::getLocation() const {
    return Location;
}
