//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTClassFunction.cpp - Class Method implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTClassMethod.h"
#include "AST/ASTClass.h"

using namespace fly;

ASTClassMethod::ASTClassMethod(const SourceLocation &Loc, ASTClassMethodKind MethodKind, ASTType *Type,
                               llvm::StringRef Name, ASTScopes *Scopes) :
        ASTFunctionBase(Loc, ASTFunctionKind::CLASS_METHOD, Type, Scopes),
        Name(Name), MethodKind(MethodKind) {

}

const StringRef &ASTClassMethod::getName() const {
    return Name;
}

ASTClass *ASTClassMethod::getClass() const {
    return Class;
}

ASTClass *ASTClassMethod::getDerivedClass() const {
    return DerivedClass;
}

bool ASTClassMethod::isConstructor() {
    return MethodKind == ASTClassMethodKind::METHOD_CONSTRUCTOR;
}

bool ASTClassMethod::isStatic() {
    return MethodKind == ASTClassMethodKind::METHOD_STATIC;
}

bool ASTClassMethod::isAbstract() const {
    return MethodKind == ASTClassMethodKind::METHOD_VIRTUAL && getBody() == nullptr;
}

CodeGenClassFunction *ASTClassMethod::getCodeGen() const {
    return CodeGen;
}

void ASTClassMethod::setCodeGen(CodeGenClassFunction *CGF) {
    CodeGen = CGF;
}

std::string ASTClassMethod::str() const {
    return Logger("ASTClassFunction").
           Super(ASTFunctionBase::str()).
            Attr("Class", Class).
            Attr("Comment", Comment).
            End();
}