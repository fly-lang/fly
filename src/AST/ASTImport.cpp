//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTImport.cpp - AST Import implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTImport.h"

#include <llvm/ADT/StringExtras.h>

using namespace fly;

ASTImport::ASTImport(const SourceLocation &Loc, llvm::SmallVector<llvm::StringRef, 4> &Names) :
        ASTBase(Loc, ASTKind::AST_IMPORT), Name(llvm::join(Names, ".")) {

}

ASTImport::~ASTImport() {

}

ASTModule * ASTImport::getModule() const {
	return Module;
}

llvm::StringRef ASTImport::getName() const {
	return  Name;
}

ASTAlias *ASTImport::getAlias() const {
    return Alias;
}

void ASTImport::setAlias(ASTAlias *Alias) {
    this->Alias = Alias;
}

std::string ASTImport::str() const {
    return Logger("ASTImport").
            Super(ASTBase::str()).
            Attr("Name", Name).
            Attr("Alias",Alias).
            End();
}
