//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaLocalVar.cpp - local variable semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaLocalVar.h"
#include "Sema/SemaVisitor.h"
#include "Sema/SemaType.h"
#include "Basic/Logger.h"

using namespace fly;

SemaLocalVar::SemaLocalVar(ASTVar &AST, SemaType *Type) : SemaVar(&AST, SemaKind::LOCAL_VAR, Type) {
}

void SemaLocalVar::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

std::string SemaLocalVar::str() const {
	return Logger("SemaLocalVar")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Name", getName())
		.Attr("Constant", isConstant())
		.Attr("Type", Type)
		.End();
}

