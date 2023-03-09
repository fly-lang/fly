//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTClass.cpp - Class implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTClass.h"
#include "AST/ASTScopes.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTClassVar.h"
#include "AST/ASTClassFunction.h"
#include "AST/ASTType.h"
#include "CodeGen/CodeGenClass.h"
#include "Basic/Debuggable.h"

using namespace fly;

ASTClass::ASTClass(ASTNode *Node, ASTClassKind ClassKind, ASTScopes *Scopes,
                   const SourceLocation &Loc, const llvm::StringRef Name,
                   llvm::SmallVector<llvm::StringRef, 4> &ExtClasses) :
        ASTTopDef(Node, ASTTopDefKind::DEF_CLASS, Scopes), ClassKind(ClassKind),
        Location(Loc), Name(Name) {
    for (llvm::StringRef ClassName : ExtClasses) {
        SuperClasses.insert(std::make_pair(ClassName, nullptr));
    }
}

llvm::StringRef ASTClass::getName() const {
    return Name;
}

const SourceLocation &ASTClass::getLocation() const {
    return Location;
}

ASTClassType *ASTClass::getType() const {
    return Type;
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

std::string ASTClass::print() const {
    std::string ClassName = Name.data();
    return NameSpace->print() + "." + ClassName;
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
