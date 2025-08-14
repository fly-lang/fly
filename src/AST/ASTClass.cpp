//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTClass.cpp - Class implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTClass.h"
#include "AST/ASTModifier.h"
#include "Basic/Logger.h"

using namespace fly;

ASTClass::ASTClass(
	ASTModule *Module, ASTClassKind ClassKind, llvm::SmallVector<ASTModifier *, 8> &Modifiers,
	const SourceLocation &Loc, llvm::StringRef Name, llvm::SmallVector<ASTTypeRef *, 4> &SuperClasses) :
	ASTBase(Loc, ASTKind::AST_CLASS), ClassKind(ClassKind), Modifiers(Modifiers), Name(Name), BaseClasses(SuperClasses) {

}

ASTModule *ASTClass::getModule() const {
	return Module;
}

llvm::SmallVector<ASTBase *, 8> ASTClass::getDefinitions() const {
	return Definitions;
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

llvm::SmallVector<ASTTypeRef *, 4> ASTClass::getBaseClasses() const {
	return BaseClasses;
}

std::string ASTClass::str() const {

	// Class to string
	return Logger("ASTClass").
	       Attr("Location", getLocation()).
	       Attr("Kind", static_cast<size_t>(getKind())).
	       Attr("ClassKind", (size_t)ClassKind).
	       Attr("Name", Name).
	       Attr("Definitions", ASTBase::str(Definitions)).
	       End();
}