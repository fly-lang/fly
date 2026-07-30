//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaArrayAlloc.cpp - reference-counted array buffer tracking
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaArrayAlloc.h"
#include "Sema/SemaVar.h"
#include "Basic/Logger.h"

using namespace fly;

SemaArrayAlloc::SemaArrayAlloc(SemaVar *Var)
    : SemaAlloc(SemaAllocKind::ARRAY), Var(Var) {}

SemaVar *SemaArrayAlloc::getVar() const {
    return Var;
}

std::string SemaArrayAlloc::str() const {
	return Logger("SemaArrayAlloc")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Var", Var)
		.End();
}
