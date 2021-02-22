//===----------------------------------------------------------------------===//
// AST/PackageDecl.cpp - Package Declaration implementation
//
// Part of the Fly Project, under the Apache License v2.0
// See https://flylang.org/LICENSE.txt for license information.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//
//
//  This file implements the Package declaration functionalities.
//
//===----------------------------------------------------------------------===//

#include "AST/PackageDecl.h"

using namespace fly;

PackageDecl::PackageDecl(const std::string &name) : name(name), type(ASTTypes::Package) {

}

PackageDecl::PackageDecl(const PackageDecl &&p) noexcept : name(p.name), type(p.type){

}
