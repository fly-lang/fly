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
#include "AST/ASTParam.h"

using namespace fly;

SemaParam::SemaParam(ASTParam &AST, SemaType *Type) : SemaVar(&AST, SemaKind::PARAM_VAR, Type) {

}

void SemaParam::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

