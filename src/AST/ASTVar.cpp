//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTVar.cpp - AST Var implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTVar.h"
#include "AST/ASTModifier.h"
#include "Basic/Logger.h"

using namespace fly;

ASTVar::ASTVar(const SourceLocation &Loc, ASTType *TypeRef, llvm::StringRef Name,
               SmallVector<ASTModifier *, 8> &Modifiers) :
        ASTNode(Loc, ASTKind::AST_VAR), Type(TypeRef), Name(Name), Modifiers(Modifiers), Sema(nullptr) {

}

ASTType *ASTVar::getType() const {
    return Type;
}

llvm::StringRef ASTVar::getName() const {
    return Name;
}

const SmallVector<ASTModifier *, 8> &ASTVar::getModifiers() const {
    return Modifiers;
}

ASTExpr * ASTVar::getExpr() const {
	return Expr;
}

void ASTVar::setExpr(ASTExpr *Expr) {
	this->Expr = Expr;
}

std::string ASTVar::str() const {
    return Logger("ASTVar").
		Attr("Location", getLocation()).
		Attr("Kind", static_cast<size_t>(getKind())).
        Attr("TypeRef", Type).
        Attr("Name", Name).
        Attr("Expr", Expr).
        Attr("Modifiers", ASTNode::str(Modifiers)).
        End();
}