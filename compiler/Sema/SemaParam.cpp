//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaParam.cpp - function parameter semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaParam.h"
#include "Sema/SemaVisitor.h"
#include "Sema/SemaType.h"
#include "Basic/Logger.h"
#include "AST/ASTParam.h"

using namespace fly;

SemaParam::SemaParam(ASTParam &AST, SemaType *Type) : SemaVar(&AST, SemaKind::PARAM_VAR, Type) {

}

void SemaParam::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

std::string SemaParam::str() const {
	return Logger("SemaParam")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Name", getName())
		.Attr("Constant", isConstant())
		.Attr("Type", Type)
		.End();
}

