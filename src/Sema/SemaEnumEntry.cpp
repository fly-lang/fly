//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaEnumEntry.cpp - The Symbolic Table for Enum Entry
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaEnumEntry.h"

using namespace fly;

SemaEnumEntry::SemaEnumEntry(ASTVar &AST, SemaType *Type) : SemaVar(&AST, SemaVarKind::ENUM_ENTRY, Type) {

}

SemaEnumEntry::~SemaEnumEntry() {
	// Delete Comment if present
	delete Comment;
}

size_t SemaEnumEntry::getIndex() const {
	return Index;
}

SemaComment * SemaEnumEntry::getComment() const {
	return Comment;
}

CodeGenEnumEntry * SemaEnumEntry::getCodeGen() const {
	return CodeGen;
}

void SemaEnumEntry::setCodeGen(CodeGenVarBase *CGV) {
	this->CodeGen = static_cast<CodeGenEnumEntry *>(CGV);
}
