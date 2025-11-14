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

ASTImport::ASTImport(ASTIdentifier *Identifier, ASTIdentifier *Alias) :
        ASTNode(Identifier->getLocation(), ASTKind::AST_IMPORT), Identifier(Identifier), Alias(Alias) {

}

ASTImport::~ASTImport() {

}

void ASTImport::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

ASTModule * ASTImport::getModule() const {
	return Module;
}

ASTIdentifier * ASTImport::getImport() const {
	return Identifier;
}

ASTIdentifier * ASTImport::getAlias() const {
	return Alias;
}

std::string ASTImport::str() const {
    return Logger("ASTImport").
	Attr("Location", getLocation()).
Attr("Kind", static_cast<size_t>(getKind())).
            Attr("Import", Identifier).
            Attr("Alias",Alias).
            End();
}
