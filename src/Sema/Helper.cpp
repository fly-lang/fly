//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/Helper.cpp - The Helper
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/Helper.h"

#include <AST/ASTIdentifier.h>

using namespace fly;

std::string Helper::Flatten(ASTIdentifier *Identifier) {
	std::string Result = Identifier->getName().str();

	while (Identifier->getParent()) {
		Result = Identifier->getParent()->getName().str() + "." + Result;
		Identifier = Identifier->getParent();
	}
}

