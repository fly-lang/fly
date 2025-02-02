//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SymFunction.cpp - The Symbolic table of Function
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sym/SymFunction.h"

using namespace fly;

SymFunction::SymFunction(ASTFunction *Function) : AST(Function) {

}

ASTFunction *SymFunction::getAST() {
    return AST;
}

SymModule * SymFunction::getModule() const {
	return Module;
}

SymComment * SymFunction::getComment() const {
	return Comment;
}

CodeGenFunction *SymFunction::getCodeGen() const {
	return CodeGen;
}

void SymFunction::setCodeGen(CodeGenFunction *CGF) {
	CodeGen = CGF;
}
