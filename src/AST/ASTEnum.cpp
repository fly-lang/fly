//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTEnum.cpp - AST Enum implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTEnum.h"
#include "AST/ASTScopes.h"

using namespace fly;

ASTEnum::ASTEnum(ASTModule *Module, const SourceLocation &Loc, llvm::StringRef Name,
                 llvm::SmallVector<ASTScope *, 8> &Scopes, llvm::SmallVector<ASTEnumType *, 4> &SuperClasses) :
        ASTBase(Loc, ASTKind::AST_ENUM), Name(Name), Scopes(Scopes), SuperClasses(SuperClasses) {

}

ASTModule * ASTEnum::getModule() const {
	return Module;
}

llvm::SmallVector<ASTBase *, 8> ASTEnum::getDefinitions() const {
	return Definitions;
}

llvm::SmallVector<ASTScope *, 8> ASTEnum::getScopes() const {
	return Scopes;
}

llvm::StringRef ASTEnum::getName() const {
	return Name;
}

llvm::SmallVector<ASTEnumType *, 4> ASTEnum::getSuperClasses() const {
	return SuperClasses;
}

std::string ASTEnum::str() const {

    // Class to string
    return Logger("ASTClass").
           Super(ASTBase::str()).
           Attr("Name", Name).
           AttrList("Scopes", Scopes).
           AttrList("Definitions", Definitions).
           End();
}
