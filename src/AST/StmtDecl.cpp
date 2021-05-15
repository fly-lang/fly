//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/FunctionDecl.cpp - Function implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/StmtDecl.h"

using namespace fly;

StmtDecl::StmtDecl(const SourceLocation &Loc, StmtDecl *Parent) : Decl(Loc), Parent(Parent) {
    if (Parent) {
        for (auto &Entry : Parent->Vars) {
            Vars.insert(std::pair<StringRef, VarDecl *>(Entry.getKey(), Entry.getValue()));
        }
    }
}

DeclKind StmtDecl::getKind() const {
    return Kind;
}

const std::vector<Decl *> &StmtDecl::getContent() const {
    return Content;
}

bool StmtDecl::isEmpty() const {
    return Content.empty();
}

const llvm::StringMap<VarDecl *> &StmtDecl::getVars() const {
    return Vars;
}

ReturnDecl *StmtDecl::getReturn() const {
    return Return;
}

bool StmtDecl::addVar(VarRefDecl *Var) {
    Content.push_back(Var);
    return true;
}

bool StmtDecl::addVar(VarDecl *Var) {
    Content.push_back(Var);
    Vars.insert(std::pair<StringRef, VarDecl *>(Var->getName(), Var));
    return true;
}

bool StmtDecl::addInvoke(FuncRefDecl *Invoke) {
    Content.push_back(Invoke);
    Invokes.insert(std::pair<StringRef, FuncRefDecl *>(Invoke->getName(), Invoke));
    return true;
}

CondStmtDecl::CondStmtDecl(const SourceLocation &Loc, StmtDecl *Parent) : StmtDecl(Loc, Parent) {}

LoopStmtDecl::LoopStmtDecl(const SourceLocation &Loc, StmtDecl *Parent) : StmtDecl(Loc, Parent) {

}

BreakDecl::BreakDecl(const SourceLocation &Loc) : Decl(Loc) {

}

DeclKind BreakDecl::getKind() const {
    return Kind;
}

ContinueDecl::ContinueDecl(const SourceLocation &Loc) : Decl(Loc) {

}

DeclKind ContinueDecl::getKind() const {
    return Kind;
}
