//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTClass.cpp - Class implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTClass.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTClassAttribute.h"
#include "AST/ASTClassMethod.h"

using namespace fly;

ASTClass::ASTClass(ASTClassKind ClassKind, ASTScopes *Scopes, const SourceLocation &Loc, llvm::StringRef Name) :
        ASTIdentity(ASTTopDefKind::DEF_CLASS, Scopes, Loc, Name), ClassKind(ClassKind) {

}

ASTClassKind ASTClass::getClassKind() const {
    return ClassKind;
}

llvm::SmallVector<ASTClassType *, 4> ASTClass::getSuperClasses() const {
    return SuperClasses;
}

llvm::StringMap<ASTClassAttribute *> ASTClass::getAttributes() const {
    return Attributes;
}

std::map <uint64_t,llvm::SmallVector <ASTClassMethod *, 4>> ASTClass::getConstructors() const {
    return Constructors;
}

llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTClassMethod *, 4>>> ASTClass::getMethods() const {
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
    llvm::SmallVector<ASTClassAttribute *, 8> VarList;
    for (auto &Field : Attributes) {
        VarList.push_back(Field.second);
    }

    // Methods to string
    llvm::SmallVector<ASTClassMethod *, 8> MethodList;
    for (auto &MapEntry : Methods) {
        for (auto &Vector : MapEntry.second) {
            for (auto &Method :  Vector.second) {
                MethodList.push_back(Method);
            }
        }
    }

    // Class to string
    return Logger("ASTClass").
           Super(ASTIdentity::str()).
           Attr("ClassKind", (uint64_t) ClassKind).
           AttrList("Vars", VarList).
           AttrList("Methods", MethodList).
           End();
}
