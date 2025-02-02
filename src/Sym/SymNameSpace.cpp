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

llvm::SmallVector<ASTModule *, 8> SymNameSpace::getModules() const {
	return Modules;
}

llvm::StringMap<SymGlobalVar *> SymNameSpace::getGlobalVars() const {
	return GlobalVars;
}

llvm::SmallVector<SymFunction *, 8> SymNameSpace::getFunctions() const {
	return Functions;
}

llvm::StringMap<SymClass *> SymNameSpace::getClasses() const {
	return Classes;
}

llvm::StringMap<SymEnum *> SymNameSpace::getEnums() const {
	return Enums;
}

CodeGenModule *SymNameSpace::getCodeGen() const {
	return CodeGen;
}

void SymNameSpace::setCodeGen(CodeGenModule *CGM) {
	CodeGen = CGM;
}
