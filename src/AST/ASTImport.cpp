//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTImport.cpp - AST Import implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTImport.h"
#include "Basic/Logger.h"

#include <AST/ASTIdentifier.h>
#include <AST/ASTVisitor.h>
#include <llvm/ADT/StringExtras.h>

using namespace fly;

ASTImport::ASTImport(const SourceLocation &Loc, llvm::SmallVector<ASTName *, 4> &Names, llvm::SmallVector<ASTName *, 4> &Alias) :
        ASTNode(Loc, ASTKind::AST_IMPORT), Names(std::move(Names)), Alias(std::move(Alias)) {

}

ASTImport::~ASTImport() {

}

void ASTImport::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

const llvm::SmallVector<ASTName *, 4> &ASTImport::getNames() const {
	return Names;
}

const llvm::SmallVector<ASTName *, 4> &ASTImport::getAlias() const {
	return Alias;
}

std::string ASTImport::str() const {
    return Logger("ASTImport").
			Attr("Kind", static_cast<size_t>(getKind())).
            // Attr("Names", Names). // FIXME: implement printing of names
            // Attr("Alias",Alias).
            End();
}
