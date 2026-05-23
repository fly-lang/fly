//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaEnumEntry.cpp - enum entry semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaEnumEntry.h"

#include "AST/ASTEnumEntry.h"
#include "Sema/SemaEnumType.h"
#include "Sema/SemaVisitor.h"

using namespace fly;

SemaEnumEntry::SemaEnumEntry(ASTEnumEntry &AST, SemaEnumType *Type) : SemaExpr(SemaKind::ENUM_ENTRY, Type), AST(AST) {

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

void SemaEnumEntry::setCodeGen(CodeGenEnumEntry *CodeGen) {
	this->CodeGen = CodeGen;
}

void SemaEnumEntry::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}
