//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/VarDecl.cpp - Var declaration Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/VarDeclStmt.h"

using namespace fly;

VarDeclStmt::VarDeclStmt(const SourceLocation &Loc, TypeBase *Type, const StringRef Name) :
        Stmt(Loc), VarDecl(Type, Name) {}

StmtKind VarDeclStmt::getKind() const {
    return Kind;
}

VarRef::VarRef(const SourceLocation &Loc, const StringRef &Name) : Name(Name) {

}

VarRef::VarRef(const SourceLocation &Loc, VarDeclStmt *D) : Name(D->getName()), Var(D) {

}

const llvm::StringRef &VarRef::getName() const {
    return Name;
}

VarDecl *VarRef::getVarDecl() const {
    return Var;
}

VarAssignStmt::VarAssignStmt(const SourceLocation &Loc, const StringRef &Name) : Stmt(Loc), VarRef(Loc, Name),
                                                                                 Expr(new GroupExpr) {

}

VarAssignStmt::VarAssignStmt(const SourceLocation &Loc, VarDeclStmt *D) : Stmt(Loc), VarRef(Loc, D),
    Expr(new GroupExpr) {

}

StmtKind VarAssignStmt::getKind() const {
    return STMT_VAR_ASSIGN;
}

GroupExpr *VarAssignStmt::getExpr() const {
    return Expr;
}
