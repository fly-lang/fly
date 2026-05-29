//===--------------------------------------------------------------------------------------------------------------===//
// compiler/AST/ASTImport.cpp - AST import declaration implementation
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
#include <AST/ASTName.h>
#include <llvm/ADT/StringExtras.h>

using namespace fly;

ASTImport::ASTImport(const SourceLocation &Loc, llvm::SmallVector<ASTName *, 4> &Names,
                     llvm::SmallVector<ASTName *, 4> &Alias, bool Wildcard) :
        ASTNode(Loc, ASTKind::AST_IMPORT), Names(std::move(Names)), Alias(std::move(Alias)), Wildcard(Wildcard) {

}

ASTImport::~ASTImport() {
    for (auto *N : Names) delete N;
    Names.clear();
    for (auto *A : Alias) delete A;
    Alias.clear();
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

bool ASTImport::isWildcard() const {
	return Wildcard;
}

std::string ASTImport::str() const {
    return Logger("ASTImport")
        .Attr("Location", getLocation())
        .Attr("Kind", static_cast<size_t>(getKind()))
        .Attr("Names", Names)
        .Attr("Alias", Alias)
        .Attr("Wildcard", Wildcard)
        .End();
}
