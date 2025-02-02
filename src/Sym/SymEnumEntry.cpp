//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SymEnumEntry.cpp - The Symbolic Table for Enum Entry
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sym/SymEnumEntry.h"

#include <strings.h>
#include <CodeGen/CodeGen.h>

using namespace fly;

SymEnumEntry::SymEnumEntry(ASTVar *AST) : SymVar(AST, SymVarKind::VAR_ENUM) {

}

uint32_t SymEnumEntry::getIndex() const {
	return Index;
}

SymComment * SymEnumEntry::getComment() const {
	return Comment;
}

CodeGenEnumEntry * SymEnumEntry::getCodeGen() const {
	return CodeGen;
}

void SymEnumEntry::setCodeGen(CodeGenEnumEntry *CodeGen) {
	this->CodeGen = CodeGen;
}
