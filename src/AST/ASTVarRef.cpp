//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTVar.cpp - Var declaration implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTVarRef.h"
#include "AST/ASTVar.h"
#include "AST/ASTIdentifier.h"
#include "Basic/Debuggable.h"

using namespace fly;

ASTVarRef::ASTVarRef(ASTIdentifier *Identifier) : ASTReference(Identifier, false) {

}

ASTVarRef::ASTVarRef(ASTVar *Var) : Def(Var), ASTReference(nullptr, false) {

}

ASTVar *ASTVarRef::getDef() const {
    return Def;
}

bool ASTVarRef::isLocalVar() {
    return Def != nullptr && Def->getVarKind() == ASTVarKind::VAR_LOCAL;
}

std::string ASTVarRef::print() const {
    return Def ? Def->print() : getIdentifier()->print();
}

std::string ASTVarRef::str() const {
    return Logger("ASTVarRef").
            Attr("Identifier", getIdentifier()).
            Attr("Instance", getInstance()).
            Attr("Def", Def).
            End();
}
