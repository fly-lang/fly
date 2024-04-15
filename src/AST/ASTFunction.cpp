//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTFunction.cpp - Function implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTFunction.h"
#include "AST/ASTScopes.h"
#include "AST/ASTNode.h"
#include "CodeGen/CodeGenFunction.h"
#include <string>

using namespace fly;

ASTFunction::ASTFunction(const SourceLocation &Loc, ASTNode *Node, ASTType *ReturnType, llvm::StringRef Name,
                         ASTScopes *Scopes) :
        ASTFunctionBase(Loc, ASTFunctionKind::FUNCTION, ReturnType, Name, Scopes), Node(Node) {

}

llvm::StringRef ASTFunction::getName() const {
    return ASTFunctionBase::getName();
}

ASTTopDefKind ASTFunction::getTopDefKind() const {
    return TopDefKind;
}

ASTNode *ASTFunction::getNode() const {
    return Node;
}

ASTNameSpace *ASTFunction::getNameSpace() const {
    return Node->getNameSpace();
}

CodeGenFunction *ASTFunction::getCodeGen() const {
    return CodeGen;
}

void ASTFunction::setCodeGen(CodeGenFunction *CGF) {
    CodeGen = CGF;
}

std::string ASTFunction::str() const {
    return Logger("ASTFunction").
            Super(ASTFunctionBase::str()).
            End();
}
