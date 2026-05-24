//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaMember.cpp - member access semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTMember.h"
#include "Sema/SemaMember.h"
#include "Sema/SemaVisitor.h"
#include "Sema/SemaType.h"
#include "Basic/Logger.h"

#include <Sema/SemaClassAttribute.h>

using namespace fly;

SemaMember::SemaMember(ASTMember &AST, SemaExpr *Ref, SemaExpr *Parent) :
	SemaExpr(SemaKind::MEMBER, Ref->getType()), AST(AST), Ref(Ref) {
	setParent(*Parent);
}

ASTMember &SemaMember::getAST() const {
	return AST;
}

SemaExpr * SemaMember::getRef() const {
	return Ref;
}

CodeGenExpr * SemaMember::getCodeGen() const {
	return CodeGen;
}

void SemaMember::setCodeGen(CodeGenExpr *CodeGen) {
	this->CodeGen = CodeGen;
}

void SemaMember::accept(SemaVisitor &Visitor) {
	Visitor.visit(*this);
}

std::string SemaMember::str() const {
	return Logger("SemaMember")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("AST", AST.str())
		.Attr("Type", Type)
		.Attr("Ref", Ref)
		.End();
}

