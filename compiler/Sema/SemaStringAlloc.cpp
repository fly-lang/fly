//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaStringAlloc.cpp - heap string allocation tracking
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaStringAlloc.h"
#include "Sema/SemaVar.h"
#include "Basic/Logger.h"

using namespace fly;

SemaStringAlloc::SemaStringAlloc(SemaVar *Var)
    : SemaAlloc(SemaAllocKind::STRING), Var(Var) {}

SemaVar *SemaStringAlloc::getVar() const {
    return Var;
}

std::string SemaStringAlloc::str() const {
	return Logger("SemaStringAlloc")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.Attr("Var", Var)
		.End();
}
