//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/Helper.cpp - The Helper
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/Helper.h"

#include <unordered_map>
#include <AST/ASTName.h>
#include <Sema/SemaType.h>

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

	switch (Type->getTypeKind()) {
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
std::string Helper::MangleFunction(llvm::StringRef Name, const llvm::SmallVector<SemaType *, 8> &Type)
{
	std::string FuncName = std::string(Name); // Function name

	// Mangling Function with _F prefix
	// Encode function name with its length
	std::string Mangled = "_F" + std::to_string(FuncName.size()) + FuncName;

	// Encode parameters
	for (const auto Param : Type) {
		Mangled += MangleType(Param);
	}

	return Mangled;
}

