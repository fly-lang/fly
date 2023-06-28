//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTDelete.cpp - Delete Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTDelete.h"
#include "AST/ASTBlock.h"
#include "AST/ASTVarRef.h"

using namespace fly;

/**
 * ASTBreak constructor
 * @param Loc
 * @param Parent
 */
ASTDelete::ASTDelete(ASTBlock *Parent, const SourceLocation &Loc, ASTVarRef *VarRef) :
        ASTStmt(Parent, Loc, ASTStmtKind::STMT_DELETE), VarRef(VarRef) {

}

ASTVarRef *ASTDelete::getVarRef() {
    return VarRef;
}

/**
 * Convert to String
 * @return string info for debugging
 */
std::string ASTDelete::str() const {
    return Logger("ASTDelete").Super(ASTStmt::str()).Attr("VarRef", VarRef->str()).End();
}
