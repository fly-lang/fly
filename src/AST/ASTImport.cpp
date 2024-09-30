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
        ASTIdentifier(Loc, Name, ASTIdentifierKind::REF_IMPORT) {

}

ASTImport::~ASTImport() {

}

ASTAlias *ASTImport::getAlias() const {
    return Alias;
}

void ASTImport::setAlias(ASTAlias *A) {
    Alias = A;
}

std::string ASTImport::str() const {
    return Logger("ASTImport").
            Super(ASTBase::str()).
            Attr("Name", Name).
            Attr("Alias",Alias).
            End();
}

ASTAlias::ASTAlias(const SourceLocation &Loc, llvm::StringRef Name) :
        ASTIdentifier(Loc, Name, ASTIdentifierKind::REF_ALIAS) {

}

std::string ASTAlias::str() const {
    return Logger("ASTImport").
            Super(ASTBase::str()).
            Attr("Name", Name).
            End();
}

ASTImport *ASTAlias::getImport() const {
    return Import;
}
