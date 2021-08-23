//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTImport.cpp - AST Namespace implementation
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


#include "AST/ASTImport.h"

using namespace fly;

ASTImport::ASTImport(const SourceLocation &Loc, llvm::StringRef Name) : Location(Loc), Name(Name), Alias(Name) {}

ASTImport::ASTImport(const SourceLocation &Loc, llvm::StringRef Name, llvm::StringRef Alias) : Location(Loc), Name(Name),
                                                                                               Alias(Alias) {}

ASTImport::~ASTImport() {
    NameSpace = nullptr;
}

const llvm::StringRef &ASTImport::getName() const {
    return Name;
}

const llvm::StringRef &ASTImport::getAlias() const {
    return Alias;
}

ASTNameSpace *ASTImport::getNameSpace() const {
    return NameSpace;
}

void ASTImport::setNameSpace(ASTNameSpace *NS) {
    NameSpace = NS;
}

const SourceLocation &ASTImport::getLocation() const {
    return Location;
}
