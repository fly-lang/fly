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
#include "AST/ASTContext.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTBlock.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTFunctionBase.h"
#include "AST/ASTFunction.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassAttribute.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTImport.h"
#include "AST/ASTClassMethod.h"
#include "AST/ASTNode.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTParam.h"
#include "Basic/Diagnostic.h"
#include "Basic/SourceLocation.h"
#include "AST/ASTBase.h"
#include "Basic/Debug.h"

#include "llvm/ADT/StringRef.h"

using namespace fly;

Sema::Sema(DiagnosticsEngine &Diags) : Diags(Diags) {

}

Sema* Sema::CreateSema(DiagnosticsEngine &Diags) {
    Sema *S = new Sema(Diags);
    S->Builder = new SemaBuilder(*S);
    S->Context = S->Builder->CreateContext();
    S->Resolver = new SemaResolver(*S);
    S->Validator = new SemaValidator(*S);
    return S;
}

DiagnosticsEngine &Sema::getDiags() const {
    return Diags;
}

SemaBuilder &Sema::getBuilder() {
    return *Builder;
}

SemaResolver &Sema::getResolver() const {
    return *Resolver;
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
    return Resolver->Resolve();
}
