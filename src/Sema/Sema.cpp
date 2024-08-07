//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/Sema.cpp - The Sema
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/Sema.h"
#include "Sema/SemaBuilder.h"
#include "Sema/SemaResolver.h"
#include "Sema/SemaValidator.h"
#include "Sema/SemaSymbols.h"
#include "Basic/Diagnostic.h"
#include "AST/ASTContext.h"

using namespace fly;

Sema::Sema(DiagnosticsEngine &Diags) : Diags(Diags) {

}

Sema* Sema::CreateSema(DiagnosticsEngine &Diags) {
    Sema *S = new Sema(Diags);
    S->Builder = new SemaBuilder(*S);
    S->Context = S->Builder->CreateContext();
    S->Validator = new SemaValidator(*S);
    S->DefaultSymbols = new SemaSymbols(*S);

    // Add default NameSpace to MapSymbols
    S->MapSymbols.insert(std::make_pair(ASTContext::DEFAULT_NAMESPACE, S->DefaultSymbols));

    return S;
}

DiagnosticsEngine &Sema::getDiags() const {
    return Diags;
}

SemaBuilder &Sema::getBuilder() {
    return *Builder;
}

SemaValidator &Sema::getValidator() const {
    return *Validator;
}

ASTContext &Sema::getContext() const {
    return *Context;
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
