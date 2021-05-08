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

DeclKind StmtDecl::getKind() const {
    return Kind;
}

StmtDecl::StmtDecl(const SourceLocation &Loc, llvm::StringMap<VarDecl*> &Map) : Decl(Loc) {
    for (auto &Entry : Map) {
        Vars.insert(std::pair<StringRef, VarDecl*>(Entry.getKey(), Entry.getValue()));
    }
}

StmtDecl::StmtDecl(const SourceLocation &Loc, std::vector<VarDecl*> &Vector) : Decl(Loc) {
    if (!Vector.empty()) {
        for (VarDecl *Var : Vector) {
            Vars.insert(std::pair<StringRef, VarDecl *>(Var->getName(), Var));
        }
    }
}

bool StmtDecl::addVar(VarRefDecl *Var) {
    Instructions.push_back(Var);
    return true;
}

bool StmtDecl::addVar(VarDecl *Var) {
    Instructions.push_back(Var);
    Vars.insert(std::pair<StringRef, VarDecl *>(Var->getName(), Var));
    return true;
}

bool StmtDecl::addInvoke(FuncRefDecl *Invoke) {
    Instructions.push_back(Invoke);
    Invokes.insert(std::pair<StringRef, FuncRefDecl *>(Invoke->getName(), Invoke));
    return true;
}
