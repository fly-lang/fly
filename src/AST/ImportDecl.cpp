//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ImportDecl.cpp - AST Namespace implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
//
// This file implements the ASTNameSpace interface.
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ImportDecl.h"

using namespace fly;

ImportDecl::ImportDecl(const SourceLocation &Loc, llvm::StringRef Name) : Location(Loc), Name(Name), Alias(Name) {}

ImportDecl::ImportDecl(const SourceLocation &Loc, llvm::StringRef Name, llvm::StringRef Alias) : Location(Loc), Name(Name),
                                                                                     Alias(Alias) {}

ImportDecl::~ImportDecl() {
    NameSpace = nullptr;
}

const llvm::StringRef &ImportDecl::getName() const {
    return Name;
}

const llvm::StringRef &ImportDecl::getAlias() const {
    return Alias;
}

ASTNameSpace *ImportDecl::getNameSpace() const {
    return NameSpace;
}

void ImportDecl::setNameSpace(ASTNameSpace *NS) {
    NameSpace = NS;
}

const SourceLocation &ImportDecl::getLocation() const {
    return Location;
}
