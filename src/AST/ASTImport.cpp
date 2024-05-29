//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTImport.cpp - AST Import implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTImport.h"

using namespace fly;

ASTImport::ASTImport(const SourceLocation &Loc, llvm::StringRef Name) :
        ASTBase(Loc), Name(Name) {

}

ASTImport::~ASTImport() {
    NameSpace = nullptr;
}

llvm::StringRef ASTImport::getName() const {
    return Name;
}

const ASTAlias *ASTImport::getAlias() const {
    return Alias;
}

void ASTImport::setAlias(ASTAlias *A) {
    Alias = A;
}

ASTNameSpace *ASTImport::getNameSpace() const {
    return NameSpace;
}

void ASTImport::setNameSpace(ASTNameSpace *NS) {
    NameSpace = NS;
}

std::string ASTImport::str() const {
    return Logger("ASTImport").
            Super(ASTBase::str()).
            Attr("Name", Name).
            Attr("NameSpace", NameSpace).
            Attr("Alias",Alias).
            End();
}

ASTAlias::ASTAlias(const SourceLocation &Loc, llvm::StringRef Name) :
        ASTBase(Loc), Name(Name) {

}

llvm::StringRef ASTAlias::getName() const {
    return Name;
}

std::string ASTAlias::str() const {
    return Logger("ASTImport").
            Super(ASTBase::str()).
            Attr("Name", Name).
            End();
}