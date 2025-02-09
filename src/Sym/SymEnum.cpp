//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SymEnum.cpp - The Sema identity Symbols
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sym/SymEnum.h"

#include "llvm/ADT/StringMap.h"

using namespace fly;

SymEnum::SymEnum(ASTEnum *AST) : SymType(SymTypeKind::TYPE_ENUM), AST(AST) {

}

ASTEnum *SymEnum::getAST() {
    return AST;
}

SymModule * SymEnum::getModule() const {
	return Module;
}

const llvm::StringMap<SymEnum *> &SymEnum::getSuperEnums() const {
	return SuperEnums;
}

SymVisibilityKind SymEnum::getVisibility() const {
	return Visibility;
}

bool SymEnum::isConstant() const {
	return Constant;
}

const llvm::StringMap<SymEnumEntry *> &SymEnum::getEntries() const {
    return Entries;
}

SymComment * SymEnum::getComment() const {
	return Comment;
}
