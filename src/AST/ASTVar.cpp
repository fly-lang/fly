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

ASTVar::ASTVar(ASTVarKind VarKind, ASTType *Type, const std::string &Name, bool Constant) :
        VarKind(VarKind), Type(Type), Name(Name), Constant(Constant) {

}

ASTVarKind ASTVar::getVarKind() {
    return VarKind;
}

ASTType *ASTVar::getType() const {
    return Type;
}

const std::string &ASTVar::getName() const {
    return Name;
}

ASTVar::~ASTVar() {
    delete Type;
}

bool ASTVar::isConstant() const {
    return Constant;
}

std::string ASTVar::str() const {
    return "Type=" + Type->str() + ", " +
           "Name=" + Name + ", " +
            "Constant=" + (Constant ? "true" : "false") + ", " +
            "VarKind=" + std::to_string(VarKind);
}

ASTVarRef::ASTVarRef(const SourceLocation &Loc, const std::string &Name, const std::string &NameSpace) :
        Loc(Loc), NameSpace(NameSpace), Name(Name) {

}

const std::string &ASTVarRef::getName() const {
    return Name;
}

ASTVar *ASTVarRef::getDef() const {
    return Def;
}

const std::string &ASTVarRef::getNameSpace() const {
    return NameSpace;
}

const SourceLocation &ASTVarRef::getLocation() const {
    return Loc;
}

std::string ASTVarRef::str() const {
    return "{ Name=" + Name + ", " +
           "NameSpace=" + NameSpace + " }";
}
