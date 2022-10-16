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

using namespace fly;

ASTVarRef::ASTVarRef(const SourceLocation &Loc, const std::string Name, const std::string NameSpace) :
        ASTVarRef(Loc, Name, "", NameSpace) {

}

ASTVarRef::ASTVarRef(const SourceLocation &Loc, const std::string Name, const std::string Class, const std::string NameSpace) :
    Loc(Loc), Name(Name), Class(Class), NameSpace(NameSpace) {
}

const std::string ASTVarRef::getName() const {
    return Name;
}

ASTVar *ASTVarRef::getDef() const {
    return Def;
}

const std::string ASTVarRef::getNameSpace() const {
    return NameSpace;
}

const std::string ASTVarRef::getClass() const {
    return Class;
}

const SourceLocation &ASTVarRef::getLocation() const {
    return Loc;
}

std::string ASTVarRef::str() const {
    return Logger("ASTVarRef").
            Attr("Location", Loc).
            Attr("NameSpace", NameSpace).
            Attr("Class", Class).
            Attr("Name", Name).
            Attr("Def", Def).
            End();
}
