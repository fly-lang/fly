//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SymTable.cpp - Sym table implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sym/SymTable.h"

using namespace fly;

/**
 * Symbol table constructor
 * @param Diags
 */
SymTable::SymTable() = default;

/**
 * Symbol table destructor
 */
SymTable::~SymTable() = default;

const llvm::DenseMap<uint64_t, SymModule *> &SymTable::getModules() const {
	return Modules;
}

const SymNameSpace * SymTable::getDefaultNameSpace() const {
	return DefaultNameSpace;
}

const llvm::StringMap<SymNameSpace *> &SymTable::getNameSpaces() const {
	return NameSpaces;
}

SymType * SymTable::getBoolType() const {
	return BoolType;
}

SymType * SymTable::getByteType() const {
	return ByteType;
}

SymType * SymTable::getUShortType() const {
	return UShortType;
}

SymType * SymTable::getShortType() const {
	return ShortType;
}

SymType * SymTable::getUIntType() const {
	return UIntType;
}

SymType * SymTable::getIntType() const {
	return IntType;
}

SymType * SymTable::getULongType() const {
	return ULongType;
}

SymType * SymTable::getLongType() const {
	return LongType;
}

SymType * SymTable::getFloatType() const {
	return FloatType;
}

SymType * SymTable::getDoubleType() const {
	return DoubleType;
}

SymType * SymTable::getVoidType() const {
	return VoidType;
}

SymType * SymTable::getCharType() const {
	return CharType;
}

SymType * SymTable::getStringType() const {
	return StringType;
}

SymType * SymTable::getErrorType() const {
	return ErrorType;
}
