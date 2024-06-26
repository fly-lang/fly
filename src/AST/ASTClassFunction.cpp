//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTClassMethod.cpp - Class Method implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTClassFunction.h"
#include "AST/ASTClass.h"

using namespace fly;

ASTClassFunction::ASTClassFunction(const SourceLocation &Loc, ASTClass *Class, ASTScopes *Scopes, ASTType *Type,
                                   llvm::StringRef Name) :
        ASTFunctionBase(Loc, ASTFunctionKind::CLASS_FUNCTION, Type, Name, Scopes), Class(Class) {

}

ASTClass *ASTClassFunction::getClass() const {
    return Class;
}

ASTClass *ASTClassFunction::getDerivedClass() const {
    return DerivedClass;
}

bool ASTClassFunction::isConstructor() {
    return Constructor;
}

bool ASTClassFunction::isStatic() {
    return Static;
}

bool ASTClassFunction::isAbstract() const {
    return getBody() == nullptr;
}

CodeGenClassFunction *ASTClassFunction::getCodeGen() const {
    return CodeGen;
}

void ASTClassFunction::setCodeGen(CodeGenClassFunction *CGF) {
    CodeGen = CGF;
}

std::string ASTClassFunction::str() const {
    return Logger("ASTClassFunction").
           Super(ASTFunctionBase::str()).
            Attr("Class", Class).
            Attr("Comment", Comment).
            End();
}
