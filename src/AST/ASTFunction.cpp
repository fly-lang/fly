//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTFunction.cpp - AST Function implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTFunction.h"
#include "AST/ASTModule.h"

using namespace fly;

ASTFunction::ASTFunction(const SourceLocation &Loc, ASTType *ReturnType, llvm::StringRef Name, ASTScopes *Scopes) :
        ASTFunctionBase(Loc, ASTFunctionKind::FUNCTION, ReturnType, Scopes), Name(Name) {

}

llvm::StringRef ASTFunction::getName() const {
    return Name;
}

ASTTopDefKind ASTFunction::getTopDefKind() const {
    return TopDefKind;
}

ASTModule *ASTFunction::getModule() const {
    return Module;
}

ASTNameSpace *ASTFunction::getNameSpace() const {
    return Module->getNameSpace();
}

CodeGenFunction *ASTFunction::getCodeGen() const {
    return CodeGen;
}

void ASTFunction::setCodeGen(CodeGenFunction *CGF) {
    CodeGen = CGF;
}

std::string ASTFunction::print() const {
    return Name.data();
}

std::string ASTFunction::str() const {
    return Logger("ASTFunction").
            Super(ASTFunctionBase::str()).
            End();
}
