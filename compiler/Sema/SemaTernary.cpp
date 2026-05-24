//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaTernary.cpp - ternary expression semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaTernary.h"
#include "Sema/SemaVisitor.h"
#include "Basic/Logger.h"

#include "AST/ASTTernary.h"
#include "Sema/Helper.h"
#include "Sema/SemaType.h"

using namespace fly;

SemaType *SemaTernary::SelectType(SemaExpr *LeftExpr, SemaExpr *RightExpr) {
	// Select the resulting type based on the operand types
	SemaType *Type1 = LeftExpr->getType();
	SemaType *Type2 = RightExpr->getType();

	// For now, we just return the type of the left expression
	if (Type1->isNumber() && Type2->isNumber()) {
		return Helper::SelectNumberType(
			static_cast<SemaNumberType *>(Type1),
			static_cast<SemaNumberType *>(Type2)
		);
	}

	// Default to Type1
	return Type1;
}

SemaTernary::SemaTernary(ASTTernary &AST, SemaExpr *Cond, SemaExpr *TrueExpr, SemaExpr *FalseExpr) :
	SemaExpr(SemaKind::TERNARY, SelectType(TrueExpr, FalseExpr)),
	AST(AST), Cond(Cond), TrueExpr(TrueExpr), FalseExpr(FalseExpr) {
}

ASTTernary &SemaTernary::getAST() const {
	return AST;
}

SemaExpr *SemaTernary::getCond() const {
	return Cond;
}

SemaExpr *SemaTernary::getTrueExpr() const {
	return TrueExpr;
}

SemaExpr *SemaTernary::getFalseExpr() const {
	return FalseExpr;
}

CodeGenExpr * SemaTernary::getCodeGen() const {
	return CodeGen;
}

void SemaTernary::setCodeGen(CodeGenExpr *CodeGen) {
	this->CodeGen = CodeGen;
}

void SemaTernary::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

std::string SemaTernary::str() const {
	return Logger("SemaTernary")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("AST", AST.str())
		.Attr("Type", Type)
		.Attr("Cond", Cond)
		.Attr("TrueExpr", TrueExpr)
		.Attr("FalseExpr", FalseExpr)
		.End();
}
