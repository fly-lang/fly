//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaEnumType.cpp - The Sema identity Symbols
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaEnumType.h"

#include "llvm/ADT/StringMap.h"

#include <AST/ASTEnum.h>

using namespace fly;

SemaEnumType::SemaEnumType(ASTEnum *AST) : SemaType(SemaTypeKind::TYPE_ENUM, AST->getName().data()), AST(AST) {

}

ASTEnum *SemaEnumType::getAST() {
    return AST;
}

SemaModule * SemaEnumType::getModule() const {
	return Module;
}

const llvm::StringMap<SemaEnumType *> &SemaEnumType::getSuperEnums() const {
	return SuperEnums;
}

SemaVisibilityKind SemaEnumType::getVisibility() const {
	return Visibility;
}

bool SemaEnumType::isConstant() const {
	return Constant;
}

const llvm::StringMap<SemaEnumEntry *> &SemaEnumType::getEntries() const {
    return Entries;
}

SemaComment * SemaEnumType::getComment() const {
	return Comment;
}
