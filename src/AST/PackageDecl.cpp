//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/PackageDecl.cpp - Package Declaration implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
//
//  This file implements the Package declaration functionalities.
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/PackageDecl.h"

using namespace fly;

PackageDecl::PackageDecl(const std::string &Name) : Name(Name), ASTType(ASTTypes::Package) {

}

PackageDecl::PackageDecl(const PackageDecl &&p) noexcept : Name(p.Name), ASTType(p.ASTType){

}
