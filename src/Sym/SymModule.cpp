//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/SymNameSpace.cpp - AST Namespace implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sym/SymModule.h"
#include "Sym/SymClass.h"
#include "Sym/SymEnum.h"

#include <llvm/IR/Module.h>

using namespace fly;

SymModule::SymModule(ASTModule *AST) : AST(AST) {

}

ASTModule* SymModule::getAST() const {
	return AST;
}

SymNameSpace *SymModule::getNameSpace() const {
	return NameSpace;
}

const llvm::StringMap<SymNameSpace *> &SymModule::getImports() const {
	return Imports;
}

llvm::StringMap<SymGlobalVar *> SymModule::getGlobalVars() const {
	return GlobalVars;
}

llvm::StringMap<SymFunction *> SymModule::getFunctions() const {
	return Functions;
}

llvm::StringMap<SymClass *> SymModule::getClasses() const {
	return Classes;
}

llvm::StringMap<SymEnum *> SymModule::getEnums() const {
	return Enums;
}