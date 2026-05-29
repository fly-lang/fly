//===--------------------------------------------------------------------------------------------------------------===//
// compiler/AST/ASTEnum.cpp - AST enum type definition implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTEnum.h"
#include "AST/ASTModifier.h"
#include "AST/ASTType.h"
#include "Basic/Logger.h"

#include <AST/ASTVisitor.h>

using namespace fly;

ASTEnum::ASTEnum(const SourceLocation &Loc, llvm::StringRef Name,
                 llvm::SmallVector<ASTModifier *, 8> &Modifiers, llvm::SmallVector<ASTType *, 4> &Bases) :
        ASTNode(Loc, ASTKind::AST_ENUM), Name(Name), Modifiers(Modifiers), Bases(Bases) {

}

ASTEnum::~ASTEnum() {
    for (auto *N : Nodes) delete N;
    Nodes.clear();
    for (auto *M : Modifiers) delete M;
    Modifiers.clear();
}

void ASTEnum::accept(ASTVisitor &Visitor) {
	Visitor.visit(*this);
}

llvm::SmallVector<ASTNode *, 8> ASTEnum::getNodes() const {
	return Nodes;
}

llvm::SmallVector<ASTModifier *, 8> ASTEnum::getModifiers() const {
	return Modifiers;
}

llvm::StringRef ASTEnum::getName() const {
	return Name;
}

llvm::SmallVector<ASTType *, 4> ASTEnum::getBases() const {
	return Bases;
}

std::string ASTEnum::str() const {
    return Logger("ASTEnum")
        .Attr("Location", getLocation())
        .Attr("Kind", static_cast<size_t>(getKind()))
        .Attr("Name", Name)
        .Attr("Modifiers", Modifiers)
        .Attr("Bases", Bases)
        .Attr("Definitions", Nodes)
        .End();
}
