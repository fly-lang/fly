//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaEnumList.cpp - The Semantic Enum List
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaEnumList.h"

#include "Sema/SemaEnumType.h"
#include "Sema/SemaVisitor.h"

using namespace fly;

SemaEnumList::SemaEnumList(SemaEnumType *EnumType, SemaType *ArrayType)
    : SemaExpr(SemaKind::ENUM_LIST, ArrayType), EnumType(EnumType) {
}

SemaEnumType *SemaEnumList::getEnumType() const {
    return EnumType;
}

void SemaEnumList::accept(SemaVisitor &Visitor) {
    Visitor.visit(*this);
}

