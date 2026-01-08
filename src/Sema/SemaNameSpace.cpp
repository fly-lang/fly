//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/SemaNameSpace.cpp - AST Namespace implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaNameSpace.h"
#include "llvm/ADT/StringRef.h"

#include <AST/ASTNameSpace.h>
#include <Sema/SymbolTable.h>

using namespace fly;

SemaNameSpace::SemaNameSpace(llvm::StringRef Name, SymbolTable *Symbols) : SemaNode(SemaKind::NAMESPACE),
	Name(Name), Symbols(Symbols) {
}

SemaNameSpace::~SemaNameSpace() {

}

SymbolTable * SemaNameSpace::getSymbols() const {
	return Symbols;
}

llvm::StringRef SemaNameSpace::getName() const {
	return Name;
}
