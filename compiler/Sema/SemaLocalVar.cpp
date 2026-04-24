//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaClassAttribute.cpp - The Symbolic Table for Class Attribute
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaLocalVar.h"
#include "Sema/SemaVisitor.h"

using namespace fly;

SemaLocalVar::SemaLocalVar(ASTVar &AST, SemaType *Type) : SemaVar(&AST, SemaKind::LOCAL_VAR, Type) {
}

void SemaLocalVar::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

