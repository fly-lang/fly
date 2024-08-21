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
#include "AST/ASTScopes.h"

using namespace fly;

ASTClassMethod::ASTClassMethod(const SourceLocation &Loc, ASTClassMethodKind MethodKind, ASTType *Type,
                               llvm::StringRef Name, llvm::SmallVector<ASTScope *, 8> &Scopes,
                               llvm::SmallVector<ASTParam *, 8> &Params) :
        ASTFunctionBase(Loc, ASTFunctionKind::CLASS_METHOD, Type, Scopes, Params),
        Name(Name), MethodKind(MethodKind), Visibility(ASTVisibilityKind::V_DEFAULT) {

}

const StringRef &ASTClassMethod::getName() const {
    return Name;
}

ASTClassMethodKind ASTClassMethod::getMethodKind() const {
    return MethodKind;
}

ASTVisibilityKind ASTClassMethod::getVisibility() const {
    return Visibility;
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
    return Static;
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
