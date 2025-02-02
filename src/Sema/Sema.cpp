//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/Sema.cpp - The Sema
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/Sema.h"
#include "Sema/ASTBuilder.h"
#include "Sema/SemaResolver.h"
#include "Sema/SemaValidator.h"
#include "Sema/SymBuilder.h"
#include "Basic/Diagnostic.h"

using namespace fly;

Sema::Sema(DiagnosticsEngine &Diags) : Diags(Diags) {

}

Sema::~Sema() {
	Modules.clear();
}

Sema* Sema::CreateSema(DiagnosticsEngine &Diags) {
    Sema *S = new Sema(Diags);

	// Init the Sema Builder
    S->Builder = new ASTBuilder(*S);

	S->SymBuildr = new SymBuilder(*S);

	// Init the Validator
    S->Validator = new SemaValidator(*S);

	// Init the Sema AST Context
	S->Table = S->SymBuildr->CreateTable();

	// Create the Default NameSpace
	S->Builder->CreateSymNameSpace();

    return S;
}

DiagnosticsEngine &Sema::getDiags() const {
    return Diags;
}

ASTBuilder &Sema::getASTBuilder() {
    return *Builder;
}

SymBuilder &Sema::getSymBuilder() {
	return *SymBuildr;
}

SemaValidator &Sema::getValidator() const {
    return *Validator;
}

SymTable &Sema::getSymTable() const {
    return *Table;
}

llvm::SmallVector<ASTModule *, 4> Sema::getModules() const {
	return Modules;
}

/**
 * Write Diagnostics
 * @param Loc
 * @param DiagID
 * @return
 */
DiagnosticBuilder Sema::Diag(SourceLocation Loc, unsigned DiagID) const {
    return Diags.Report(Loc, DiagID);
}

/**
 * Write Diagnostics
 * @param Loc
 * @param DiagID
 * @return
 */
DiagnosticBuilder Sema::Diag(unsigned DiagID) const {
    return Diags.Report(DiagID);
}

bool Sema::Resolve() {
    return SemaResolver::Resolve(*this);
}
