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
#include <Sema/SemaBuilder.h>
#include <Sema/SemaBuiltin.h>
#include <Sema/SymbolTable.h>

using namespace fly;

std::string Registry::DEFAULT_NAMESPACE = "default";

Registry::Registry() : GlobalScope(CreateBuiltinScope()), DefaultNameSpace(new SemaNameSpace(DEFAULT_NAMESPACE)) {
	NameSpaces.insert(std::make_pair<>(DefaultNameSpace->getName(), DefaultNameSpace));
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
	Builtin->insert(new Symbol(BoolType->getName(), SemaKind::BUILTIN_TYPE, BoolType));
	Builtin->insert(new Symbol(ByteType->getName(), SemaKind::BUILTIN_TYPE, ByteType));
	Builtin->insert(new Symbol(UShortType->getName(), SemaKind::BUILTIN_TYPE, UShortType));
	Builtin->insert(new Symbol(ShortType->getName(), SemaKind::BUILTIN_TYPE, ShortType));
	Builtin->insert(new Symbol(UIntType->getName(), SemaKind::BUILTIN_TYPE, UIntType));
	Builtin->insert(new Symbol(IntType->getName(), SemaKind::BUILTIN_TYPE, IntType));
	Builtin->insert(new Symbol(ULongType->getName(), SemaKind::BUILTIN_TYPE, ULongType));
	Builtin->insert(new Symbol(LongType->getName(), SemaKind::BUILTIN_TYPE, LongType));
	Builtin->insert(new Symbol(FloatType->getName(), SemaKind::BUILTIN_TYPE, FloatType));
	Builtin->insert(new Symbol(DoubleType->getName(), SemaKind::BUILTIN_TYPE, DoubleType));
	Builtin->insert(new Symbol(StringType->getName(), SemaKind::BUILTIN_TYPE, StringType));
	Builtin->insert(new Symbol(VoidType->getName(), SemaKind::BUILTIN_TYPE, VoidType));
	Builtin->insert(new Symbol(ErrorType->getName(), SemaKind::BUILTIN_TYPE, ErrorType));

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

SemaNameSpace * Registry::getOrAddFQNameSpace(const std::string &Name, SemaNameSpace *Parent) {
	// Create the CurrentNameSpace if not exists yet in the Context
	auto I = NameSpaces.find(Name);
	if (I != NameSpaces.end()) {
		return I->second;
	} else {
		auto NameSpace = new SemaNameSpace(Name, Parent);
		NameSpaces.insert(std::make_pair<>(NameSpace->getName(), NameSpace));
		return NameSpace;
	}
}

SemaNameSpace * Registry::getFQNameSpace(const std::string &FQName) {
	auto I = NameSpaces.find(FQName);
	if (I != NameSpaces.end()) {
		return I->second;
	}
	return nullptr;
}

llvm::SmallVector<LocalScope, 4> Registry::getBodies() const {
	return Bodies;
}

void Registry::addBody(SymbolTable* Symbols, ASTBlockStmt *Body) {
	Bodies.push_back(LocalScope{Symbols, Body});
}

SemaNameSpace* Registry::getNameSpace(std::string Name) {
	auto I = NameSpaces.find(Name);
	if (I != NameSpaces.end()) {
		return I->second;
	}
	return nullptr;
}

void Registry::addNameSpace(SemaNameSpace *NameSpace) {
	NameSpaces.insert(std::make_pair(NameSpace->getName(), NameSpace));
}

SemaType* Registry::LookupBuiltinType(llvm::StringRef Ref) {
	return static_cast<SemaType *>(BuiltinScope->lookup(Ref)->getRef());
}

SymbolTable* Registry::LookupInNameSpaces(llvm::StringRef Ref) {
	auto It = NameSpaces.find(Ref.str());
	if (It != NameSpaces.end()) {
		return It->second->getSymbols();
	}
	return nullptr;
}
