//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTType.cpp - AST Type Ref implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTType.h"
#include "Basic/Logger.h"

#include <AST/ASTVisitor.h>

using namespace fly;

ASTType::ASTType(const SourceLocation &Loc, llvm::StringRef Name, bool BuiltIn) :
        ASTRef(Loc, Name, ASTRefKind::REF_TYPE), BuiltIn(BuiltIn) {

}

void ASTType::accept(ASTVisitor& Visitor) {
	Visitor.visit(*this);
}

SemaType * ASTType::getSema() const {
	return Sema;
}

void ASTType::setSema(SemaType *Sema) {
	this->Sema = Sema;
}

ASTNameSpaceRef *ASTType::getNameSpaceRef() const {
	return NameSpaceRef;
}

std::string ASTType::str() const {
    return Logger("ASTType").
	Attr("Location", getLocation()).
	Attr("Kind", static_cast<size_t>(getKind())).
    End();
}

ASTArrayType::ASTArrayType(const SourceLocation &Loc, ASTType *ElementType, llvm::StringRef Name) :
	ASTType(Loc, Name, ElementType->getNameSpaceRef()) {
}

ASTType * ASTArrayType::getElementType() const {
	return ElementType;
}

ASTExpr * ASTArrayType::getSizeExpr() const {
	return SizeExpr;
}

std::string ASTArrayType::str() const {
	return ASTType::str();
}
