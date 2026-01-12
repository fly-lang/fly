//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/Helper.cpp - The Helper
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

SemaIntType *Helper::SelectIntType(SemaExpr *Expr1, SemaExpr *Expr2) {
	// Promotes First or Second Expr Types in order to be equal
	SemaIntType * Type1 = static_cast<SemaIntType *>(Expr1->getType());
	SemaIntType * Type2 = static_cast<SemaIntType *>(Expr2->getType());
	if (Type1->getIntKind() > Type2->getIntKind())
		return Type1;
	return Type2;
}

SemaFloatType *Helper::SelectFloatType(SemaExpr *Expr1, SemaExpr *Expr2) {
	// Promotes First or Second Expr Types in order to be equal
	SemaFloatType * Type1 = static_cast<SemaFloatType *>(Expr1->getType());
	SemaFloatType * Type2 = static_cast<SemaFloatType *>(Expr2->getType());
	if (Type1->getFPKind() > Type2->getFPKind())
		return Type1;
	return Type2;
}

