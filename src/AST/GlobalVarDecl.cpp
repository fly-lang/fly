//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/GlobalVarDecl.cpp - Global Var declaration implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/GlobalVarDecl.h"

using namespace fly;

GlobalVarDecl::GlobalVarDecl(SourceLocation &Loc, TypeBase *Type, StringRef Name) : TopDecl(Loc),
    VarDecl(Type, Name, true) {

}

TopDeclKind GlobalVarDecl::getKind() const {
    return Kind;
}

CGGlobalVar *GlobalVarDecl::getCodeGen() const {
    return CodeGen;
}

void GlobalVarDecl::setCodeGen(CGGlobalVar *codeGen) {
    CodeGen = codeGen;
}
