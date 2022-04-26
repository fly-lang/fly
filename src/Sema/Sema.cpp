//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/Sema.cpp - GlobalVar Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/Sema.h"
#include "AST/ASTBlock.h"
#include "AST/ASTVar.h"
#include "Basic/Diagnostic.h"

using namespace fly;

Sema::Sema(ASTNode *AST) : AST(AST) {

}

bool Sema::CheckUndefVar(ASTBlock *Block, ASTVarRef *VarRef) {
    if (Block->HasUndefVar(VarRef)) {
        Block->Diag(VarRef->getLocation(), diag::err_undef_var) << VarRef->getName();
        return false;
    }
    return true;
}

bool Sema::CheckOnCloseBlock(ASTBlock *Block) {

    return false;
}
