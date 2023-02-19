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

ASTVarRef::ASTVarRef(ASTIdentifier *Identifier) : Identifier(Identifier) {

}

ASTVarRef::ASTVarRef(ASTVar *Var) : Def(Var), Instance(Var) {

}

SourceLocation ASTVarRef::getLocation() const {
    return Identifier ? Identifier->getLocation() : SourceLocation();
}

llvm::StringRef ASTVarRef::getName() const {
    return Identifier ? Identifier->getName() : llvm::StringRef();
}

ASTIdentifier *ASTVarRef::getIdentifier() const {
    return Identifier;
}

ASTVar *ASTVarRef::getDef() const {
    return Def;
}

ASTVar *ASTVarRef::getInstance() const {
    return Instance;
}

bool ASTVarRef::isLocalVar() {
    return Def != nullptr && Def->getVarKind() == ASTVarKind::VAR_LOCAL;
}

std::string ASTVarRef::print() const {
    return Def ? Def->print() : Identifier->print();
}

std::string ASTVarRef::str() const {
    return Logger("ASTVarRef").
            Attr("Identifier", Identifier).
            Attr("Instance", Instance).
            Attr("Def", Def).
            End();
}
