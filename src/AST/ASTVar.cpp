//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTVar.cpp - Var declaration implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTVar.h"

using namespace fly;

ASTVar::ASTVar(ASTType *Type, const StringRef &Name, const StringRef &NameSpace) :
    Type(Type), Name(Name), NameSpace(NameSpace) {}

ASTType *ASTVar::getType() const {
    return Type;
}

const llvm::StringRef &ASTVar::getName() const {
    return Name;
}

ASTVar::~ASTVar() {
    delete Type;
}

bool ASTVar::isConstant() const {
    return Constant;
}

const bool ASTVar::isGlobal() const {
    return !NameSpace.empty();
}


ASTVarRef::ASTVarRef(const SourceLocation &Loc, const llvm::StringRef &Name, const StringRef &NameSpace) :
        Loc(Loc), NameSpace(NameSpace), Name(Name) {

}

const llvm::StringRef &ASTVarRef::getName() const {
    return Name;
}

ASTVar *ASTVarRef::getDecl() const {
    return Decl;
}

void ASTVarRef::setDecl(ASTVar *D) {
    Decl = D;
}

const StringRef &ASTVarRef::getNameSpace() const {
    return NameSpace;
}

const SourceLocation &ASTVarRef::getLocation() const {
    return Loc;
}

unsigned long ASTVarRef::getOrder() const {
    return Order;
}

void ASTVarRef::setOrder(unsigned long order) {
    Order = order;
}