//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/SymNameSpace.cpp - AST Namespace implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sym/SymNameSpace.h"
#include "llvm/ADT/StringRef.h"

using namespace fly;

std::string SymNameSpace::DEFAULT_NAMESPACE = "default";

SymNameSpace::SymNameSpace() : Name(DEFAULT_NAMESPACE) {
}

SymNameSpace::SymNameSpace(llvm::StringRef Name) : Name(Name) {
}

SymNameSpace::~SymNameSpace() = default;

llvm::StringRef SymNameSpace::getName() const {
	return Name;
}

const llvm::SmallVector<ASTModule *, 8> &SymNameSpace::getModules() const {
	return Modules;
}

const llvm::StringMap<SymGlobalVar *> &SymNameSpace::getGlobalVars() const {
	return GlobalVars;
}

const llvm::StringMap<SymFunction *> &SymNameSpace::getFunctions() const {
	return Functions;
}

const llvm::StringMap<SymType *> &SymNameSpace::getTypes() const {
	return Types;
}

CodeGenModule *SymNameSpace::getCodeGen() const {
	return CodeGen;
}

void SymNameSpace::setCodeGen(CodeGenModule *CGM) {
	CodeGen = CGM;
}
