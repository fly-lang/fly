//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/Registry.cpp - The Namespace Registry
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "Sema/Registry.h"
#include "Sema/SemaNameSpace.h"

#include <AST/ASTNameSpace.h>
#include <Sema/SemaBuiltin.h>
#include <Sema/SemaFunction.h>
#include <Sema/SymbolTable.h>

using namespace fly;

std::string Registry::DEFAULT_NAMESPACE = "default";

Registry::Registry() : GlobalScope(CreateBuiltinScope()), DefaultNameSpace(new SemaNameSpace(DEFAULT_NAMESPACE)) {
	NameSpaces.insert(std::make_pair<>(DefaultNameSpace->getName(), DefaultNameSpace));
}

Registry::~Registry() {
	// Delete Builtin Scope
	delete BuiltinScope;

	// Delete Global Scope
	delete GlobalScope;

	// Delete all Namespaces
	for (auto &Pair : NameSpaces) {
		delete Pair.second;
	}
}

SymbolTable* Registry::CreateBuiltinScope() {
	SymbolTable* Builtin = new SymbolTable(nullptr);

	auto BoolType = SemaBuiltin::getBoolType();
	auto ByteType = SemaBuiltin::getByteType();
	auto UShortType = SemaBuiltin::getUShortType();
	auto ShortType = SemaBuiltin::getShortType();
	auto UIntType = SemaBuiltin::getUIntType();
	auto IntType = SemaBuiltin::getIntType();
	auto ULongType = SemaBuiltin::getULongType();
	auto LongType = SemaBuiltin::getLongType();
	auto FloatType = SemaBuiltin::getFloatType();
	auto DoubleType = SemaBuiltin::getDoubleType();
	auto StringType = SemaBuiltin::getStringType();
	auto VoidType = SemaBuiltin::getVoidType();
	auto ErrorType = SemaBuiltin::getErrorType();

	// Insert Builtin Types
	Builtin->insert(new Symbol(BoolType->getName(), SemaKind::TYPE, BoolType));
	Builtin->insert(new Symbol(ByteType->getName(), SemaKind::TYPE, ByteType));
	Builtin->insert(new Symbol(UShortType->getName(), SemaKind::TYPE, UShortType));
	Builtin->insert(new Symbol(ShortType->getName(), SemaKind::TYPE, ShortType));
	Builtin->insert(new Symbol(UIntType->getName(), SemaKind::TYPE, UIntType));
	Builtin->insert(new Symbol(IntType->getName(), SemaKind::TYPE, IntType));
	Builtin->insert(new Symbol(ULongType->getName(), SemaKind::TYPE, ULongType));
	Builtin->insert(new Symbol(LongType->getName(), SemaKind::TYPE, LongType));
	Builtin->insert(new Symbol(FloatType->getName(), SemaKind::TYPE, FloatType));
	Builtin->insert(new Symbol(DoubleType->getName(), SemaKind::TYPE, DoubleType));
	Builtin->insert(new Symbol(StringType->getName(), SemaKind::TYPE, StringType));
	Builtin->insert(new Symbol(VoidType->getName(), SemaKind::TYPE, VoidType));
	Builtin->insert(new Symbol(ErrorType->getName(), SemaKind::TYPE, ErrorType));

	return Builtin;
}

void Registry::addModule(SemaModule *Module) {
	Modules.push_back(Module);
}

llvm::SmallVector<SemaModule *, 8> &Registry::getModules() {
	return Modules;
}

SemaNameSpace * Registry::getDefaultNameSpace() {
	return DefaultNameSpace;
}

llvm::SmallVector<LocalScope, 4> Registry::getBodies() const {
	return Bodies;
}

void Registry::addBody(SymbolTable* Symbols, ASTBlockStmt *Body) {
	Bodies.push_back(LocalScope{Symbols, Body});
}

SemaNameSpace *Registry::getNameSpace(const llvm::SmallVector<ASTName *, 4> &Names) {
	SemaNameSpace *Current = nullptr;
	auto Children = NameSpaces;

	for (int i = 0; i < Names.size(); i++) {
		llvm::StringRef Name = Names[i]->getName();
		auto It = Children.find(Name);
		if (It != Children.end()) {
			Current = It->second;
			Children = Current->getChildren();
		} else {
			return nullptr; // not found
		}
	}

	return Current;
}

SemaNameSpace* Registry::getOrCreateNameSpace(const llvm::SmallVector<ASTName *, 4>& Names) {
	if (Names.empty())
		return DefaultNameSpace;

	auto Children = NameSpaces;
	SemaNameSpace *Current = nullptr;
	SemaNameSpace * Parent = nullptr;

	for (auto *N : Names) {
		llvm::StringRef Name = N->getName();

		auto It = Children.find(Name);
		if (It == Children.end()) {
			// Create namespace
			Current = new SemaNameSpace(Name, Parent);
			Children[Name] = Current;
		} else {
			// Namespace already exists
			Current = It->second;
		}
		// Advance deeper
		Parent   = Current;
		Children = Current->getChildren();
	}

	return Current;
}

SemaType* Registry::LookupBuiltinType(llvm::StringRef Ref) {
	return static_cast<SemaType *>(BuiltinScope->lookup(Ref)->getRef());
}

SemaType* Registry::LookupNamedType(llvm::StringRef Name, SemaNameSpace *NameSpace) {
	Symbol * Sym = NameSpace->getSymbols()->lookup(Name);
	if (Sym && Sym->getKind() == SemaKind::TYPE) {
		return static_cast<SemaType *>(Sym->getRef());
	}
	return nullptr; // not found
}

SemaType * Registry::LookupNamedType(ASTNamedType &NamedType, SemaNameSpace *NameSpace) {
	const SmallVector<ASTName *, 4> &Names = NamedType.getNames();
	SemaNameSpace * CurrentNameSpace = NameSpace;
	auto &Children = NameSpaces;
	for (int i = 0; i < Names.size(); i++) {
		llvm::StringRef Name = Names[i]->getName();

		// Look for Type
		if (i == Names.size()-1) {
			return LookupNamedType(Name, CurrentNameSpace);
		}

		// Look for Namespace
		auto It = Children.find(Name);
		if (It != Children.end()) {
			CurrentNameSpace = It->second;
			Children = CurrentNameSpace->getChildren();
		} else {
			return nullptr; // not found
		}
	}
	return nullptr;
}

SemaNameSpace* Registry::LookupNameSpace(llvm::StringRef Name, SemaNameSpace *NameSpace) {
	llvm::StringMap<SemaNameSpace *> Search;
	if (NameSpace)
		Search = NameSpace->getChildren();
	else
		Search = NameSpaces;

	auto It = Search.find(Name);
	if (It != Search.end()) {
		return It->second;
	}
	return nullptr; // not found
}

SemaFunction* Registry::LookupFunction(llvm::StringRef MangledName, SemaNameSpace* NameSpace) {
    Symbol * Sym = NameSpace->getSymbols()->lookup(MangledName);
    if (Sym && (Sym->getKind() == SemaKind::FUNCTION)) {
        return static_cast<SemaFunction *>(Sym->getRef());
    }
    return nullptr; // not found
}