//===--------------------------------------------------------------------------------------------------------------===//
// compiler/CodeGen/CodeGenHelper.cpp - code generation helper utilities
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenHelper.h"

#include "AST/ASTFunction.h"
#include "AST/ASTName.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTParam.h"
#include "AST/ASTType.h"
#include "Sema/SemaFunctionBase.h"
#include "Sema/SemaParam.h"
#include "Sema/SemaType.h"
#include "Sema/Symbol.h"

#include <unordered_map>

using namespace fly;

static std::unordered_map<std::string, std::string> MangleTypeMap = {
	{"bool", "_b"},
	{"byte", "_y"},
	{"ushort", "_us"},
	{"short", "_s"},
	{"uint", "_ui"},
	{"int", "_i"},
	{"ulong", "_ul"},
	{"long", "_l"},
	{"ptrsize", "_pz"},
	{"float", "_f"},
	{"double", "_d"},
	{"void", "_v"},
	{"string", "_Ss"},
	{"error", "_e"},
};

std::string CodeGenHelper::Mangle(SemaType *T) {
	std::string Mangled;
	switch (T->getKind()) {
		case SemaKind::TYPE_ARRAY: {
			SemaArrayType *Array = static_cast<SemaArrayType *>(T);
			Mangled += "_A" + Mangle(Array->getElementType());
		} break;
		case SemaKind::TYPE_ENUM:
			Mangled += "_E" + T->getName();
			break;
		case SemaKind::TYPE_CLASS:
			Mangled += "_C" + T->getName();
			break;
		default:
			Mangled += MangleTypeMap.at(T->getName());
	}
	return Mangled;
}

std::string CodeGenHelper::Mangle(SemaFunctionBase *F) {
	std::string FuncName = std::string(F->getName());
	const std::string &NS = F->getNamespaceName();
	std::string Mangled = "_F";
	if (!NS.empty())
		Mangled += std::to_string(NS.size()) + NS;
	Mangled += std::to_string(FuncName.size()) + FuncName;
	for (const auto Param : F->getParams())
		Mangled += Mangle(Param->getType());
	return Mangled;
}

std::string CodeGenHelper::Mangle(ASTType *T) {
	switch (T->getTypeKind()) {
		case ASTTypeKind::TYPE_BUILTIN: {
			auto *BT = static_cast<ASTBuiltinType *>(T);
			switch (BT->getBuiltinKind()) {
				case ASTBuiltinTypeKind::TYPE_BOOL:   return "_b";
				case ASTBuiltinTypeKind::TYPE_BYTE:   return "_y";
				case ASTBuiltinTypeKind::TYPE_SHORT:  return "_s";
				case ASTBuiltinTypeKind::TYPE_USHORT: return "_us";
				case ASTBuiltinTypeKind::TYPE_INT:    return "_i";
				case ASTBuiltinTypeKind::TYPE_UINT:   return "_ui";
				case ASTBuiltinTypeKind::TYPE_LONG:   return "_l";
				case ASTBuiltinTypeKind::TYPE_ULONG:  return "_ul";
				case ASTBuiltinTypeKind::TYPE_FLOAT:  return "_f";
				case ASTBuiltinTypeKind::TYPE_DOUBLE: return "_d";
				case ASTBuiltinTypeKind::TYPE_VOID:   return "_v";
				case ASTBuiltinTypeKind::TYPE_STRING: return "_Ss";
				case ASTBuiltinTypeKind::TYPE_ERROR:  return "_e";
				default: return "";
			}
		}
		case ASTTypeKind::TYPE_NAMED: {
			auto *NT = static_cast<ASTNamedType *>(T);
			std::string Name = NT->getNames().back()->getName().str();
			Symbol *Sym = T->getSymbol();
			if (Sym && Sym->getKind() == SymbolKind::ENUM)
				return "_E" + Name;
			return "_C" + Name;
		}
		case ASTTypeKind::TYPE_ARRAY: {
			auto *AT = static_cast<ASTArrayType *>(T);
			return "_A" + Mangle(AT->getElementType());
		}
	}
	return "";
}

std::string CodeGenHelper::FlattenNS(ASTNameSpace *NS) {
	if (!NS) return "";
	std::string Result;
	for (auto *N : NS->getNames()) {
		if (!Result.empty()) Result += "_";
		Result += N->getName().str();
	}
	return Result;
}

std::string CodeGenHelper::Mangle(const std::string &NS, ASTFunction *F) {
	std::string M = "_F";
	if (!NS.empty()) M += std::to_string(NS.size()) + NS;
	std::string Name = F->getName().str();
	M += std::to_string(Name.size()) + Name;
	for (auto *P : F->getParams())
		M += Mangle(P->getType());
	return M;
}

std::string CodeGenHelper::Mangle(const std::string &NS, const std::string &ClassName, ASTFunction *F) {
	return ClassName + Mangle(NS, F);
}

std::string CodeGenHelper::Mangle(ASTNameSpace *NS, ASTFunction *F) {
	return Mangle(FlattenNS(NS), F);
}

std::string CodeGenHelper::Mangle(ASTNameSpace *NS, const std::string &ClassName, ASTFunction *F) {
	return ClassName + Mangle(FlattenNS(NS), F);
}
