//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaFunctionBase.cpp - The Symbolic table of Function Base
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaFunctionBase.h"
#include "Sema/SemaParam.h"

#include <unordered_map>
#include <AST/ASTFunction.h>
#include <AST/ASTTypeRef.h>
#include <AST/ASTVar.h>
#include <CodeGen/CodeGenFunctionBase.h>
#include <Sema/SemaErrorHandler.h>
#include <Sema/SemaType.h>

using namespace fly;

SemaFunctionBase::SemaFunctionBase(ASTFunction *AST, SemaFunctionKind Kind, std::string MangledName) :
	AST(AST), Kind(Kind), MangledName(MangledName), ErrorHandler(new SemaErrorHandler()) {

}

std::string SemaFunctionBase::getMangledName() const {
	return MangledName;
}

llvm::SmallVector<SemaParam *, 8> &SemaFunctionBase::getParams() {
    return Params;
}

SemaType *SemaFunctionBase::getReturnType() {
    return ReturnType;
}

ASTFunction * SemaFunctionBase::getAST() {
	return AST;
}

SemaFunctionKind SemaFunctionBase::getKind() const {
    return Kind;
}

llvm::SmallVector<SemaVar *, 8> SemaFunctionBase::getLocalVars() {
	return LocalVars;
}

SemaErrorHandler * SemaFunctionBase::getErrorHandler() const {
	return ErrorHandler;
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
std::string MangleType(SemaType *Type) {
	std::string Mangled = "";

	switch (Type->getKind()) {
	case SemaTypeKind::TYPE_ARRAY: {
		SemaArrayType *Array = static_cast<SemaArrayType *>(Type);
		Mangled += "_A" + MangleType(Array->getType());
	}	break;
	case SemaTypeKind::TYPE_CLASS:
		Mangled += "_C" + Type->getName();
		break;
	case SemaTypeKind::TYPE_ENUM: {
		Mangled += "_E" + Type->getName();
	}	break;
	default:
		Mangled += typeMap.at(Type->getName());
	}

	return Mangled;
}

// Function to generate a unique mangled function name
std::string SemaFunctionBase::MangleFunction(llvm::StringRef Name, const llvm::SmallVector<SemaType *, 8> &Params)
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
