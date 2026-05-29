//===--------------------------------------------------------------------------------------------------------------===//
// compiler/AST/ASTClass.cpp - AST class definition implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTClass.h"
#include "AST/ASTModifier.h"
#include "AST/ASTType.h"
#include "Basic/Logger.h"

#include <AST/ASTIdentifier.h>
#include <AST/ASTVisitor.h>

using namespace fly;

ASTClass::ASTClass(ASTClassKind ClassKind, llvm::SmallVector<ASTModifier *, 8> &Modifiers,
	const SourceLocation &Loc, llvm::StringRef Name, llvm::SmallVector<ASTType *, 4> &Bases) :
	ASTNode(Loc, ASTKind::AST_CLASS), ClassKind(ClassKind), Modifiers(Modifiers), Name(Name), Bases(Bases) {

}

ASTClass::~ASTClass() {
    for (auto *N : Nodes) delete N;
    Nodes.clear();
    for (auto *M : Modifiers) delete M;
    Modifiers.clear();
}

void ASTClass::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

llvm::SmallVector<ASTNode *, 8> ASTClass::getNodes() const {
	return Nodes;
}

ASTClassKind ASTClass::getClassKind() const {
	return ClassKind;
}

llvm::SmallVector<ASTModifier *, 8> ASTClass::getModifiers() const {
	return Modifiers;
}

llvm::StringRef ASTClass::getName() const {
	return Name;
}

const llvm::SmallVector<ASTType *, 4> &ASTClass::getBases() const {
	return Bases;
}

const llvm::SmallVector<ASTTypeParam *, 4> &ASTClass::getTypeParams() const {
	return TypeParams;
}

std::string ASTClass::str() const {
    return Logger("ASTClass")
        .Attr("Location", getLocation())
        .Attr("Kind", static_cast<size_t>(getKind()))
        .Attr("ClassKind", static_cast<size_t>(ClassKind))
        .Attr("Name", Name)
        .Attr("TypeParams", TypeParams)
        .Attr("Modifiers", Modifiers)
        .Attr("Bases", Bases)
        .Attr("Definitions", Nodes)
        .End();
}