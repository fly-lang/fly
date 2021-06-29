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

void VarDecl::setExpr(GroupExpr *Exp) {
    Expression = Exp;
}


VarRef::VarRef(const SourceLocation &Loc, const StringRef &Name) :
        Loc(Loc), NameSpace(""), Name(Name) {

}

VarRef::VarRef(const SourceLocation &Loc, const llvm::StringRef &NameSpace, const StringRef &Name) :
        Loc(Loc), NameSpace(NameSpace), Name(Name) {

}

const llvm::StringRef &VarRef::getName() const {
    return Name;
}

VarDecl *VarRef::getDecl() const {
    return Var;
}

void VarRef::setDecl(VarDecl *D) {
    Var = D;
}

const StringRef &VarRef::getNameSpace() const {
    return NameSpace;
}

const SourceLocation &VarRef::getLocation() const {
    return Loc;
}

VarStmt::VarStmt(const SourceLocation &Loc, BlockStmt *Block, const StringRef &Name) : Stmt(Loc, Block),
    VarRef(Loc, Name), Expr(new GroupExpr) {

}

StmtKind VarStmt::getKind() const {
    return STMT_VAR_ASSIGN;
}

GroupExpr *VarStmt::getExpr() const {
    return Expr;
}

void VarStmt::setExpr(GroupExpr *Exp) {
    Expr = Exp;
}