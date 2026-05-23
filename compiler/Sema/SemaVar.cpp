//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaVar.cpp - variable semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaVar.h"
#include "Sema/SemaSmartAlloc.h"
#include "Sema/SemaStringAlloc.h"
#include <AST/ASTVar.h>

using namespace fly;

SemaVar::SemaVar(ASTVar *AST, SemaKind Kind, SemaType *Type) :
	SemaExpr(Kind, Type), AST(AST) {
}

SemaVar::~SemaVar() {
	// CodeGen: deleted by parent SemaExpr destructor.
	// Alloc: NOT deleted here — owned by SemaBlockStmt::Allocs.
}

ASTVar *SemaVar::getAST() const {
	return AST;
}

llvm::StringRef SemaVar::getName() const {
	return AST->getName();
}

bool SemaVar::isConstant() const {
	return Constant;
}

SemaAlloc *SemaVar::getAlloc() const {
	return Alloc;
}

void SemaVar::setAlloc(SemaAlloc *A) {
	Alloc = A;
}

SemaSmartAlloc *SemaVar::getSmartAlloc() const {
	if (Alloc && Alloc->getKind() == SemaAllocKind::SMART)
		return static_cast<SemaSmartAlloc *>(Alloc);
	return nullptr;
}

SemaStringAlloc *SemaVar::getStringAlloc() const {
	if (Alloc && Alloc->getKind() == SemaAllocKind::STRING)
		return static_cast<SemaStringAlloc *>(Alloc);
	return nullptr;
}

CodeGenVar * SemaVar::getCodeGen() const {
	return static_cast<CodeGenVar *>(CodeGen);
}

void SemaVar::setCodeGen(CodeGenVar *CodeGen) {
	this->CodeGen = CodeGen;
}
