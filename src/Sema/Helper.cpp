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

// Mapping Fly types to mangled representations
std::unordered_map<std::string, std::string> MangleTypeMap = {
	{"bool", "_b"},
	{"byte", "_y"},
	{"ushort", "_us"},
	{"short", "_s"},
	{"uint", "_ui"},
	{"int", "_i"},
	{"ulong", "_ul"},
	{"long", "_l"},
	{"float", "_f"},
	{"double", "_d"},
	{"void", "_v"},
	{"string", "_Ss"},
	// {"char", "_c"},
	{"error", "_e"},
};

// Mapping Fly types to mangled representations
std::unordered_map<ASTBuiltinTypeKind, std::string> BuiltinTypeMap = {
	{ASTBuiltinTypeKind::TYPE_BOOL, "_b"},
	{ASTBuiltinTypeKind::TYPE_BYTE, "_y"},
	{ASTBuiltinTypeKind::TYPE_USHORT, "_us"},
	{ASTBuiltinTypeKind::TYPE_SHORT, "_s"},
	{ASTBuiltinTypeKind::TYPE_UINT, "_ui"},
	{ASTBuiltinTypeKind::TYPE_INT, "_i"},
	{ASTBuiltinTypeKind::TYPE_ULONG, "_ul"},
	{ASTBuiltinTypeKind::TYPE_LONG, "_l"},
	{ASTBuiltinTypeKind::TYPE_FLOAT, "_f"},
	{ASTBuiltinTypeKind::TYPE_DOUBLE, "_d"},
	{ASTBuiltinTypeKind::TYPE_VOID, "_v"},
	{ASTBuiltinTypeKind::TYPE_STRING, "_Ss"},
	// {ASTBuiltinTypeKind::TYPE_CHAR, "_c"},
	{ASTBuiltinTypeKind::TYPE_ERROR, "_e"},
};

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

// Function to process array type: "int[5]" -> "A5_i"
// std::string Helper::MangleType(SemaType *Type) {
// 	std::string Mangled = "";
//
// 	switch (Type->getTypeKind()) {
// 		case SemaTypeKind::TYPE_ARRAY: {
// 			SemaArrayType *Array = static_cast<SemaArrayType *>(Type);
// 			Mangled += "_A" + MangleType(Array->getType());
// 		}	break;
// 		case SemaTypeKind::TYPE_ENUM:
// 		case SemaTypeKind::TYPE_CLASS:
// 			Mangled += "_O" + Type->getName();
// 			break;
// 		default:
// 			Mangled += MangleTypeMap.at(Type->getName());
// 	}
//
// 	return Mangled;
// }

// std::string Helper::MangleType(ASTType *Type) {
// 	std::string Mangled = "";
//
// 	switch (Type->getTypeKind()) {
//
// 		case ASTTypeKind::TYPE_NAMED:
// 			Mangled += "_O" + Flatten(static_cast<ASTNamedType *>(Type)->getNames());
// 			break;
// 		case ASTTypeKind::TYPE_BUILTIN:
// 			Mangled += BuiltinTypeMap.at(static_cast<ASTBuiltinType *>(Type)->getBuiltinKind());
// 			break;
// 		case ASTTypeKind::TYPE_ARRAY:
// 			Mangled += "_A" + MangleType(static_cast<ASTArrayType *>(Type)->getElementType());
// 			break;
// 	}
// 	return Mangled;
// }
//
// std::string Helper::Mangle(ASTCall *AST) {
// 	std::string Name = std::string(AST->getName()); // Function name
//
// 	// Mangling Function with _F prefix
// 	// Encode function name with its length
// 	std::string Mangled = "_F" + std::to_string(Name.size()) + Name;
//
// 	// Encode parameters
// 	for (const auto Arg : AST->getArgs()) {
// 		Mangled += MangleType(Arg->getExpr()->getType());
// 	}
//
// 	return Mangled;
// }
//
// std::string Helper::Mangle(ASTFunction *AST) {
// 	std::string Name = std::string(AST->getName()); // Function name
//
// 	// Mangling Function with _F prefix
// 	// Encode function name with its length
// 	std::string Mangled = "_F" + std::to_string(Name.size()) + Name;
//
// 	// Encode parameters
// 	for (const auto Param : AST->getParams()) {
// 		Mangled += MangleType(Param->getType());
// 	}
//
// 	return Mangled;
// }
