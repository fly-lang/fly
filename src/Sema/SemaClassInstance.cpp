//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaClassInstance.cpp - The Symbolic Table for Class Attribute
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaClassInstance.h"
#include "CodeGen/CodeGenVar.h"

#include <Sema/SemaClassType.h>

using namespace fly;

SemaClassInstance::SemaClassInstance(SemaClassType *Class) :
	SemaVar(nullptr, SemaVarKind::CLASS_INSTANCE), Class(Class) {
		SemaResult::Type = Class;
}

SemaClassType * SemaClassInstance::getClass() const {
	return Class;
}

CodeGenVarBase * SemaClassInstance::getCodeGen() const {
	return CodeGen;
}

void SemaClassInstance::setCodeGen(CodeGenVarBase *CodeGen) {
	this->CodeGen = static_cast<CodeGenVar *>(CodeGen);
}
