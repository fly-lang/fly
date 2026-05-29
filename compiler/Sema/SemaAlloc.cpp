//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaAlloc.cpp - allocation semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaAlloc.h"
#include "Basic/Logger.h"

using namespace fly;

std::string SemaAlloc::str() const {
	return Logger("SemaAlloc")
		.Attr("Kind", static_cast<uint64_t>(getKind()))
		.End();
}
