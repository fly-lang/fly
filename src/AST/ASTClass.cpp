//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTClass.cpp - Class implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTClass.h"
#include "AST/ASTScopes.h"

using namespace fly;

ASTClass::ASTClass(ASTModule *Module, ASTClassKind ClassKind, llvm::SmallVector<ASTScope *, 8> &Scopes,
                   const SourceLocation &Loc, llvm::StringRef Name, llvm::SmallVector<ASTTypeRef *, 4> &SuperClasses) :
        ASTBase(Loc, ASTKind::AST_CLASS), ClassKind(ClassKind), Scopes(Scopes), Name(Name), SuperClasses(SuperClasses) {

}

ASTModule * ASTClass::getModule() const {
	return Module;
}

llvm::SmallVector<ASTBase *, 8> ASTClass::getDefinitions() const {
	return Definitions;
}

ASTClassKind ASTClass::getClassKind() const {
	return ClassKind;
}

llvm::SmallVector<ASTScope *, 8> ASTClass::getScopes() const {
	return Scopes;
}

llvm::StringRef ASTClass::getName() const {
	return Name;
}

llvm::SmallVector<ASTTypeRef *, 4> ASTClass::getSuperClasses() const {
    return SuperClasses;
}

std::string ASTClass::str() const {

    // Class to string
    return Logger("ASTClass").
           Super(ASTBase::str()).
           Attr("ClassKind", (size_t) ClassKind).
           Attr("Name", Name).
           AttrList("Definitions", Definitions).
           End();
}
