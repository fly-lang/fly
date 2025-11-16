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

ASTType::ASTType(const SourceLocation &Loc) :
        ASTNode(Loc, ASTKind::AST_TYPE) {

}

SemaType * ASTType::getSema() const {
	return Sema;
}

void ASTType::setSema(SemaType *Sema) {
	this->Sema = Sema;
}

ASTArrayType::ASTArrayType(const SourceLocation &Loc, ASTType *ElementType, ASTExpr *Size) :
	ASTType(Loc), ElementType(ElementType), Size(Size) {
}

ASTType * ASTArrayType::getElementType() const {
	return ElementType;
}

ASTExpr * ASTArrayType::getSizeExpr() const {
	return Size;
}

std::string ASTArrayType::str() const {
	return Logger("ASTArrayType").
		Attr("Location", getLocation()).
		Attr("Kind", static_cast<size_t>(getKind())).
		End();
}

ASTBuiltinType::ASTBuiltinType(const SourceLocation &Loc, ASTBuiltinTypeKind Kind) : ASTType(Loc), BuiltinKind(Kind) {
}

void ASTBuiltinType::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

std::string ASTBuiltinType::str() const {
	return ASTType::str();
}

ASTNamedType::ASTNamedType(const SourceLocation &Loc, llvm::StringRef Name) : ASTType(Loc), Name(Name) {
}

void ASTNamedType::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

llvm::StringRef ASTNamedType::getName() const {
	return Name;
}

std::string ASTNamedType::str() const {
	return ASTType::str();
}

void ASTArrayType::accept(ASTVisitor &Visitor) {
}
