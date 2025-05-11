//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/SemaNameSpace.cpp - AST Namespace implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaModule.h"
#include "Sema/SemaClassType.h"
#include "Sema/SemaEnumType.h"

#include <llvm/IR/Module.h>

using namespace fly;

SemaModule::SemaModule(ASTModule *AST) : AST(AST) {

}

ASTModule* SemaModule::getAST() const {
	return AST;
}

SemaNameSpace *SemaModule::getNameSpace() const {
	return NameSpace;
}

const llvm::StringMap<SemaNameSpace *> &SemaModule::getImports() const {
	return Imports;
}

const llvm::StringMap<SemaGlobalVar *> &SemaModule::getGlobalVars() const {
	return GlobalVars;
}

const llvm::StringMap<SemaFunction *> &SemaModule::getFunctions() const {
	return Functions;
}

const llvm::StringMap<SemaType *> &SemaModule::getTypes() const {
	return Types;
}
