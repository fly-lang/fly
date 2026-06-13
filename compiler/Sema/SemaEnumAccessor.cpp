//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaEnumAccessor.cpp - enum accessor semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaEnumAccessor.h"

#include "Basic/Logger.h"
#include "Sema/SemaEnumType.h"
#include "Sema/SemaVisitor.h"

using namespace fly;

SemaEnumAccessor::SemaEnumAccessor(SemaEnumType *EnumType, SemaEnumEntry *Entry,
                                   SemaVar *Var, bool IsName, SemaType *Type)
    : SemaExpr(SemaKind::ENUM_ACCESSOR, Type), EnumType(EnumType),
      Entry(Entry), Var(Var), IsName(IsName) {
}

SemaEnumType *SemaEnumAccessor::getEnumType() const {
    return EnumType;
}

SemaEnumEntry *SemaEnumAccessor::getEntry() const {
    return Entry;
}

SemaVar *SemaEnumAccessor::getVar() const {
    return Var;
}

bool SemaEnumAccessor::isName() const {
    return IsName;
}

void SemaEnumAccessor::accept(SemaVisitor &Visitor) {
    Visitor.visit(*this);
}

std::string SemaEnumAccessor::str() const {
	return Logger("SemaEnumAccessor")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("EnumType", EnumType)
		.Attr("IsName", IsName)
		.Attr("Type", Type)
		.End();
}
