//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTEnum.cpp - AST Enum implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTEnum.h"
#include "AST/ASTModifier.h"
#include "Basic/Logger.h"

using namespace fly;

ASTEnum::ASTEnum(ASTModule *Module, const SourceLocation &Loc, llvm::StringRef Name,
                 llvm::SmallVector<ASTModifier *, 8> &Modifiers, llvm::SmallVector<ASTTypeRef *, 4> &SuperClasses) :
        ASTNode(Loc, ASTKind::AST_ENUM), Name(Name), Modifiers(Modifiers), SuperClasses(SuperClasses) {

}

ASTModule * ASTEnum::getModule() const {
	return Module;
}

llvm::SmallVector<ASTNode *, 8> ASTEnum::getDefinitions() const {
	return Definitions;
}

llvm::SmallVector<ASTModifier *, 8> ASTEnum::getModifiers() const {
	return Modifiers;
}

llvm::StringRef ASTEnum::getName() const {
	return Name;
}

llvm::SmallVector<ASTTypeRef *, 4> ASTEnum::getSuperClasses() const {
	return SuperClasses;
}

std::string ASTEnum::str() const {

    // Class to string
    return Logger("ASTClass").
	Attr("Location", getLocation()).
 Attr("Kind", static_cast<size_t>(getKind())).
           Attr("Name", Name).
           Attr("Modifiers", ASTNode::str(Modifiers)).
           Attr("Definitions", ASTNode::str(Definitions)).
           End();
}
