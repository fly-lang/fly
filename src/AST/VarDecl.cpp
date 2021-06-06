//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/VarDecl.h - Var declaration implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/VarDecl.h"

using namespace fly;

VarDecl::VarDecl(TypeBase *Type, const StringRef &Name, bool isGlobal) : Type(Type), Name(Name), Global(isGlobal) {}

TypeBase *VarDecl::getType() const {
    return Type;
}

const llvm::StringRef &VarDecl::getName() const {
    return Name;
}

VarDecl::~VarDecl() {
    delete Type;
    delete Expression;
}

bool VarDecl::isConstant() const {
    return Constant;
}

const bool VarDecl::isGlobal() const {
    return Global;
}

GroupExpr *VarDecl::getExpr() const {
    return Expression;
}


VarRef::VarRef(const SourceLocation &Loc, const StringRef &Name) : Name(Name) {

}

VarRef::VarRef(const SourceLocation &Loc, VarDecl *D) : Name(D->getName()), Var(D) {

}

const llvm::StringRef &VarRef::getName() const {
    return Name;
}

VarDecl *VarRef::getVarDecl() const {
    return Var;
}

VarStmt::VarStmt(const SourceLocation &Loc, BlockStmt *CurrStmt, const StringRef &Name) : Stmt(Loc, CurrStmt),
    VarRef(Loc, Name), Expr(new GroupExpr) {

}

VarStmt::VarStmt(const SourceLocation &Loc, BlockStmt *CurrStmt, VarDecl *D) : Stmt(Loc, CurrStmt), VarRef(Loc, D),
                                                          Expr(new GroupExpr) {

}

StmtKind VarStmt::getKind() const {
    return STMT_VAR_ASSIGN;
}

GroupExpr *VarStmt::getExpr() const {
    return Expr;
}