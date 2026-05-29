//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaEnumList.cpp - enum list semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaEnumList.h"

#include "Basic/Logger.h"
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

std::string SemaEnumList::str() const {
	return Logger("SemaEnumList")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("EnumType", EnumType)
		.Attr("Type", Type)
		.End();
}

