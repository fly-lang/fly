//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SymFunctionBase.cpp - The Symbolic table of Function Base
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sym/SymFunctionBase.h"

#include <unordered_map>
#include <AST/ASTFunction.h>
#include <AST/ASTTypeRef.h>
#include <AST/ASTVar.h>
#include <CodeGen/CodeGenFunctionBase.h>
#include <Sym/SymType.h>

using namespace fly;

SymFunctionBase::SymFunctionBase(ASTFunction *AST, SymFunctionKind Kind) : AST(AST), Kind(Kind) {
}

llvm::SmallVector<SymType *, 8> &SymFunctionBase::getParamTypes() {
    return ParamTypes;
}

SymType *SymFunctionBase::getReturnType() {
    return ReturnType;
}

ASTFunction * SymFunctionBase::getAST() {
	return AST;
}

SymFunctionKind SymFunctionBase::getKind() const {
    return Kind;
}

// Mapping Fly types to mangled representations
std::unordered_map<std::string, std::string> typeMap = {
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
	{"char", "_c"},
	{"error", "_e"},
	// Class _C
	// Enum _E
	// Function _F
	// Array _A
};

// Function to process array type: "int[5]" -> "A5_i"
std::string MangleType(SymType *Type) {
	std::string Mangled = "";

	switch (Type->getKind()) {
	case SymTypeKind::TYPE_ARRAY: {
		SymTypeArray *Array = static_cast<SymTypeArray *>(Type);
		Mangled += "_A" + MangleType(Array->getType());
	}	break;
	case SymTypeKind::TYPE_CLASS:
		Mangled += "_C" + Type->getName();
		break;
	case SymTypeKind::TYPE_ENUM: {
		Mangled += "_E" + Type->getName();
	}	break;
	default:
		Mangled += typeMap.at(Type->getName());
	}

	return Mangled;
}

// Function to mangle a type reference
std::string SymFunctionBase::MangleFunction(ASTFunction *AST) {
	llvm::SmallVector<SymType *, 8> Params;
	for (auto Param : AST->getParams()) {
		Params.push_back(Param->getTypeRef()->getType());
	}
	return MangleFunction(AST->getName(), Params);
}

// Function to generate a unique mangled function name
std::string SymFunctionBase::MangleFunction(llvm::StringRef Name, const llvm::SmallVector<SymType *, 8> &Params)
{
	std::string FuncName = std::string(Name); // Function name

	// Mangling Function with _F prefix
	// Encode function name with its length
	std::string Mangled = "_F" + std::to_string(FuncName.size()) + FuncName;

	// Encode parameters
	for (const auto Param : Params) {
		Mangled += MangleType(Param);
	}

	return Mangled;
}
