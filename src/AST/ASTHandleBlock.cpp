//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTHandleBlock.cpp - If Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTHandleBlock.h"

using namespace fly;

ASTHandleBlock::ASTHandleBlock(ASTBlock *Parent, const SourceLocation &Loc, ASTVarRef *ErrorRef) :
        ASTBlock(Parent, Loc, ASTBlockKind::BLOCK_HANDLE), ErrorRef(ErrorRef) {

}

ASTBlock *ASTHandleBlock::getParent() const {
    return (ASTBlock *) Parent;
}

ASTVarRef *ASTHandleBlock::getErrorRef() const {
    return ErrorRef;
}

std::string ASTHandleBlock::str() const {
    return Logger("ASTHandleBlock").
           Super(ASTBlock::str()).
            Attr("Error", ErrorRef).
           End();
}
