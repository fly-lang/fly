//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/Decl.cpp - Declaration Base
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/Decl.h"

using namespace fly;

Decl::Decl(const SourceLocation &Loc) : Location(Loc) {}

const SourceLocation &Decl::getLocation() const {
    return Location;
}

VisibilityKind TopDecl::getVisibility() const {
    return Visibility;
}
