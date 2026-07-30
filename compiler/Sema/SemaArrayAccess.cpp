//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaArrayAccess.cpp - resolved array subscript
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaArrayAccess.h"
#include "Sema/SemaVisitor.h"
#include "Sema/SemaType.h"
#include "Basic/Logger.h"
#include "AST/ASTArrayAccess.h"

using namespace fly;

SemaArrayAccess::SemaArrayAccess(ASTArrayAccess &AST, SemaExpr *Base, SemaExpr *Index, SemaType *Type) :
	SemaExpr(SemaKind::ARRAY_ACCESS, Type), AST(AST), Base(Base), Index(Index) {
}

ASTArrayAccess &SemaArrayAccess::getAST() const {
	return AST;
}

SemaExpr *SemaArrayAccess::getBase() const {
	return Base;
}

SemaExpr *SemaArrayAccess::getIndex() const {
	return Index;
}

CodeGenExpr *SemaArrayAccess::getCodeGen() const {
	return CodeGen;
}

void SemaArrayAccess::setCodeGen(CodeGenExpr *CodeGen) {
	this->CodeGen = CodeGen;
}

void SemaArrayAccess::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

std::string SemaArrayAccess::str() const {
	return Logger("SemaArrayAccess")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Type", Type)
		.Attr("Base", Base)
		.Attr("Index", Index)
		.End();
}
