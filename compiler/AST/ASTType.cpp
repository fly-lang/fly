//===-------------------------------------------------------------------------------------------------------------===//
// compiler/AST/ASTType.cpp - AST type reference implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTType.h"
#include "AST/ASTExpr.h"
#include "AST/ASTName.h"
#include "Basic/Logger.h"
#include "Sema/Symbol.h"

#include <AST/ASTVisitor.h>

using namespace fly;

ASTType::ASTType(const SourceLocation &Loc, ASTTypeKind TypeKind) :
        ASTNode(Loc, ASTKind::AST_TYPE), TypeKind(TypeKind) {
}

ASTTypeKind ASTType::getTypeKind() const { return TypeKind; }

Symbol *ASTType::getSymbol() const { return Sym; }

void ASTType::setSymbol(Symbol *S) { Sym = S; }


ASTArrayType::ASTArrayType(const SourceLocation &Loc, ASTType *ElementType, ASTExpr *Size) :
	ASTType(Loc, ASTTypeKind::TYPE_ARRAY), ElementType(ElementType), Size(Size) {
}

void ASTArrayType::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

ASTType * ASTArrayType::getElementType() const {
	return ElementType;
}

ASTExpr * ASTArrayType::getSizeExpr() const {
	return Size;
}

std::string ASTArrayType::str() const {
    return Logger("ASTArrayType")
        .Attr("Location", getLocation())
        .Attr("Kind", static_cast<size_t>(getKind()))
        .Attr("ElementType", ElementType)
        .Attr("Size", Size)
        .End();
}

ASTBuiltinType::ASTBuiltinType(const SourceLocation &Loc, ASTBuiltinTypeKind Kind) :
	ASTType(Loc,ASTTypeKind::TYPE_BUILTIN), BuiltinKind(Kind) {
}

ASTBuiltinTypeKind ASTBuiltinType::getBuiltinKind() const {
	return BuiltinKind;
}

void ASTBuiltinType::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

std::string ASTBuiltinType::str() const {
    return Logger("ASTBuiltinType")
        .Attr("Location", getLocation())
        .Attr("Kind", static_cast<size_t>(getKind()))
        .Attr("BuiltinKind", static_cast<size_t>(BuiltinKind))
        .End();
}

ASTNamedType::ASTNamedType(const SourceLocation &Loc, llvm::SmallVector<ASTName *, 4> Names) :
	ASTType(Loc, ASTTypeKind::TYPE_NAMED), Names(Names) {
}

void ASTNamedType::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

const llvm::SmallVector<ASTName *, 4> &ASTNamedType::getNames() const {
	return Names;
}

std::string ASTNamedType::str() const {
    return Logger("ASTNamedType")
        .Attr("Location", getLocation())
        .Attr("Kind", static_cast<size_t>(getKind()))
        .Attr("Names", Names)
        .End();
}
