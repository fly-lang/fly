//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SymFunction.cpp - The Symbolic table of Function
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sym/SymFunction.h"
#include "Sym/SymType.h"
#include <unordered_map>
#include <vector>
#include <AST/ASTFunction.h>
#include <AST/ASTTypeRef.h>
#include <AST/ASTVar.h>
#include <Sym/SymType.h>

using namespace fly;

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
		case SymTypeKind::TYPE_ARRAY:
			SymTypeArray *Array = static_cast<SymTypeArray *>(Type);
			Mangled += "_A" + Array->getSize() + MangleType(Array->getType());
			break;
		case SymTypeKind::TYPE_CLASS:
			Mangled += "_C" + Type->getName();
			break;
		case SymTypeKind::TYPE_ENUM:
			Mangled += "_E" + Type->getName();
			break;
		default:
			Mangled += typeMap.at(Type->getName());
	}

	return Mangled;
}

// Function to generate a unique mangled function name
std::string MangleFunction(ASTFunction *AST)
{
	std::string Name = std::string(AST->getName()); // Function name

	// Mangling Function with _F prefix
	// Encode function name with its length
	std::string Mangled = "_F" + std::to_string(Name.size()) + Name;

	// Encode parameters
	for (const auto Param : AST->getParams()) {
		Mangled += MangleType(Param->getTypeRef()->getDef());
	}

	return Mangled;
}


SymFunction::SymFunction(ASTFunction *AST) : SymFunctionBase(AST, SymFunctionKind::FUNCTION), MangledName(Mangle(AST)) {

}

SymModule * SymFunction::getModule() const {
	return Module;
}

std::string SymFunction::getMangledName() const {
	return MangledName;
}

SymComment * SymFunction::getComment() const {
	return Comment;
}

SymVisibilityKind SymFunction::getVisibility() const {
	return Visibility;
}

CodeGenFunction *SymFunction::getCodeGen() const {
	return CodeGen;
}

void SymFunction::setCodeGen(CodeGenFunction *CGF) {
	CodeGen = CGF;
}
