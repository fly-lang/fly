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

ASTVar::ASTVar(ASTType *Type, const StringRef &Name, const StringRef &NameSpaceStr, bool Global) :
        Type(Type), Name(Name), NameSpaceStr(NameSpaceStr), Global(Global) {}

ASTType *ASTVar::getType() const {
    return Type;
}

const llvm::StringRef &ASTVar::getName() const {
    return Name;
}

const llvm::StringRef &ASTVar::getPrefix() const {
    return NameSpaceStr;
}

ASTVar::~ASTVar() {
    delete Type;
}

bool ASTVar::isConstant() const {
    return Constant;
}

bool ASTVar::isGlobal() const {
    return Global;
}

std::string ASTVar::str() const {
    return "Type=" + Type->str() + ", " +
           "NameSpace=" + NameSpaceStr.str() + ", " +
           "Name=" + Name.str() + ", " +
            "Constant=" + (Constant ? "true" : "false") + ", " +
            "Global=" + (Global ? "true" : "false");
}

ASTVarRef::ASTVarRef(const SourceLocation &Loc, const llvm::StringRef &Name, const StringRef &NameSpace) :
        Loc(Loc), NameSpace(NameSpace), Name(Name) {

}

ASTVarRef::ASTVarRef(ASTVar *Var) : ASTVarRef(Loc, Var->getName(), Var->getPrefix()) {
    Decl = Var;
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

std::string ASTVarRef::str() const {
    return "{ Name=" + Name.str() + ", " +
           "NameSpace=" + NameSpace.str() + " }";
}
