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
#include "AST/ASTBlock.h"
#include "Sema/SemaBuilder.h"
#include "CodeGen/CodeGenClass.h"

using namespace fly;

ASTClass::ASTClass(ASTNode *Node, ASTClassKind ClassKind, ASTScopes *Scopes,
                   const SourceLocation &Loc, llvm::StringRef Name,
                   llvm::SmallVector<ASTClassType *, 4> &ExtClasses) :
        ASTIdentity(Node, ASTTopDefKind::DEF_CLASS, Scopes, Loc, Name), ClassKind(ClassKind),
        SuperClasses(ExtClasses) {

}

ASTClassType *ASTClass::getType() {
    return Type;
}

ASTClassKind ASTClass::getClassKind() const {
    return ClassKind;
}

llvm::SmallVector<ASTClassType *, 4> ASTClass::getSuperClasses() const {
    return SuperClasses;
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
    return getNameSpace()->print() + "." + ClassName;
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
           Super(ASTIdentity::str()).
           Attr("ClassKind", (uint64_t) ClassKind).
           AttrList("Vars", VarList).
           AttrList("Methods", MethodList).
           End();
}
