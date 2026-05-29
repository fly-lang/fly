//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/Helper.cpp - sema helper utilities
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/Helper.h"

#include "AST/ASTArg.h"
#include "AST/ASTCall.h"
#include "AST/ASTFunction.h"
#include "AST/ASTParam.h"
#include "Sema/SemaBuiltin.h"
#include "Sema/SemaFunctionBase.h"

#include <AST/ASTName.h>
#include <Sema/SemaType.h>
#include <unordered_map>

using namespace fly;

std::string Helper::Flatten(llvm::SmallVector<ASTName *, 4> Names) {
	if (Names.empty())
		return "";

	std::string Result;
	Result.append(Names[0]->getName().str());
	for (size_t i = 1; i < Names.size(); ++i) {
		Result.push_back('.');
		Result.append(Names[i]->getName().str());
	}

	return Result;
}

SemaIntType *Helper::SelectIntType(SemaIntType *Type1, SemaIntType *Type2) {
	// Promotes First or Second Expr Types in order to be equal
	if (Type1->getIntKind() > Type2->getIntKind())
		return Type1;
	return Type2;
}

SemaFloatType *Helper::SelectFloatType(SemaFloatType *Type1, SemaFloatType *Type2) {
	// Promotes First or Second Expr Types in order to be equal
	if (Type1->getFloatKind() > Type2->getFloatKind())
		return Type1;
	return Type2;
}

SemaType *Helper::SelectNumberType(SemaNumberType *Type1, SemaNumberType *Type2) {
	// Follow numeric tower: int -> float
	// If either operand is float, result is float (take the higher float type)
	if (Type1->isFloat() && Type2->isFloat()) {
		// Both are float types, select the higher one
		return SelectFloatType(static_cast<SemaFloatType *>(Type1), static_cast<SemaFloatType *>(Type2));
	}
	if (Type1->isFloat()) {
		// Left is float, right is int - use left (float type)
		return Type1;
	}
	if (Type2->isFloat()) {
		// Right is float, left is int - use right (float type)
		return Type2;
	}
	// Both are integer types, select the higher one
	return SelectIntType(static_cast<SemaIntType *>(Type1), static_cast<SemaIntType *>(Type2));
}