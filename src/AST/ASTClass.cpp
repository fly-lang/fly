//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTClass.cpp - Class implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTClass.h"
#include "AST/ASTClassVar.h"
#include "AST/ASTClassFunction.h"
#include "CodeGen/CodeGenClass.h"
#include "Basic/Debuggable.h"

using namespace fly;

ASTClassScopes::ASTClassScopes(ASTClassVisibilityKind Visibility, bool Constant) :
        Visibility(Visibility), Constant(Constant){

}

ASTClassVisibilityKind ASTClassScopes::getVisibility() const {
    return Visibility;
}

bool ASTClassScopes::isConstant() const {
    return Constant;
}

std::string ASTClassScopes::str() const {
    return Logger("ASTClassScopes").
            Attr("Visibility", (uint64_t) Visibility).
            Attr("Constant", Constant).
            End();
}

ASTClass::ASTClass(const SourceLocation &Loc, ASTNode *Node, llvm::StringRef Name, ASTTopScopes *Scopes) :
        ASTTopDef(Node, ASTTopDefKind::DEF_CLASS, Scopes), Name(Name), Location(Loc) {

}

llvm::StringRef ASTClass::getName() const {
    return Name;
}

const SourceLocation &ASTClass::getLocation() const {
    return Location;
}

ASTClassKind ASTClass::getClassKind() const {
    return ClassKind;
}

llvm::StringMap<ASTClassVar *> ASTClass::getVars() const {
    return Vars;
}

std::map <uint64_t,llvm::SmallVector <ASTClassFunction *, 4>> ASTClass::getConstructors() const {
    return Constructors;
}

llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTClassFunction *, 4>>> ASTClass::getMethods() const {
    return Methods;
}

CodeGenClass *ASTClass::getCodeGen() const {
    return CodeGen;
}

void ASTClass::setCodeGen(CodeGenClass *CGC) {
    CodeGen = CGC;
}

std::string ASTClass::str() const {

    // Fields to string
    std::vector<ASTClassVar *> VarList;
    for (auto &Field : Vars) {
        VarList.push_back(Field.second);
    }

    // Methods to string
    std::vector<ASTClassFunction *> MethodList;
    for (auto &MapEntry : Methods) {
        for (auto &Vector : MapEntry.second) {
            for (auto &Method :  Vector.second) {
                MethodList.push_back(Method);
            }
        }
    }

    // Class to string
    return Logger("ASTClass").
           Super(ASTTopDef::str()).
           Attr("Name", Name).
           Attr("Scopes", Scopes).
           Attr("ClassKind", (uint64_t) ClassKind).
           AttrList("Vars", VarList).
           AttrList("Methods", MethodList).
           End();
}
