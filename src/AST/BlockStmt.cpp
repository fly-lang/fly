//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/BlockStmt.cpp - Block Statement implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/BlockStmt.h"

using namespace fly;

BlockStmt::BlockStmt(const SourceLocation &Loc, BlockStmt *Parent) : Stmt(Loc), Parent(Parent) {
    if (Parent) {
        for (auto &Entry : Parent->Vars) {
            Vars.insert(std::pair<llvm::StringRef, VarDeclStmt *>(Entry.getKey(), Entry.getValue()));
        }
    }
}

StmtKind BlockStmt::getKind() const {
    return Kind;
}

const std::vector<Stmt *> &BlockStmt::getContent() const {
    return Content;
}

bool BlockStmt::isEmpty() const {
    return Content.empty();
}

const llvm::StringMap<VarDeclStmt *> &BlockStmt::getVars() const {
    return Vars;
}

ReturnStmt *BlockStmt::getReturn() const {
    return Return;
}

bool BlockStmt::addVar(VarAssignStmt *Var) {
    Content.push_back(Var);
    return true;
}

bool BlockStmt::addVar(VarDeclStmt *Var) {
    Content.push_back(Var);
    Vars.insert(std::pair<StringRef, VarDeclStmt *>(Var->getName(), Var));
    return true;
}

bool BlockStmt::addInvoke(FuncCallStmt *Invoke) {
    Content.push_back(Invoke);
    FuncCalls.insert(std::pair<StringRef, FuncCallStmt *>(Invoke->getName(), Invoke));
    return true;
}

ConditionBlockStmt::ConditionBlockStmt(const SourceLocation &Loc, BlockStmt *Parent) : BlockStmt(Loc, Parent) {}

LoopBlockStmt::LoopBlockStmt(const SourceLocation &Loc, BlockStmt *Parent) : BlockStmt(Loc, Parent) {

}

BreakStmt::BreakStmt(const SourceLocation &Loc) : Stmt(Loc) {

}

StmtKind BreakStmt::getKind() const {
    return Kind;
}

ContinueStmt::ContinueStmt(const SourceLocation &Loc) : Stmt(Loc) {

}

StmtKind ContinueStmt::getKind() const {
    return Kind;
}
