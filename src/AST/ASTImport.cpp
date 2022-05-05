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
#include "AST/ASTNameSpace.h"

using namespace fly;

ASTImport::ASTImport(const SourceLocation &NameLoc, const std::string Name,
                     const SourceLocation &AliasLoc, const std::string Alias) :
        NameLocation(NameLoc), Name(Name), AliasLocation(AliasLoc), Alias(Alias) {

}

ASTImport::~ASTImport() {
    NameSpace = nullptr;
}

const std::string &ASTImport::getName() const {
    return Name;
}

const std::string &ASTImport::getAlias() const {
    return Alias;
}

ASTNameSpace *ASTImport::getNameSpace() const {
    return NameSpace;
}

void ASTImport::setNameSpace(ASTNameSpace *NS) {
    NameSpace = NS;
}

const SourceLocation &ASTImport::getNameLocation() const {
    return NameLocation;
}

const SourceLocation &ASTImport::getAliasLocation() const {
    return AliasLocation;
}

std::string ASTImport::str() const {
    return "{ Name=" + Name +
            ", NameSpace=" + (NameSpace ? NameSpace->str() : "{}") +
            ", Alias=" + Alias +
            " }";
}
