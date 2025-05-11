//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SymTable.cpp - Sym table implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SymTable.h"

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

const llvm::DenseMap<uint64_t, SemaModule *> &SymTable::getModules() const {
	return Modules;
}

SemaNameSpace * SymTable::getDefaultNameSpace() const {
	return DefaultNameSpace;
}

const llvm::StringMap<SemaNameSpace *> &SymTable::getNameSpaces() const {
	return NameSpaces;
}

SemaType * SymTable::getBoolType() const {
	return BoolType;
}

SemaType * SymTable::getByteType() const {
	return ByteType;
}

SemaType * SymTable::getUShortType() const {
	return UShortType;
}

SemaType * SymTable::getShortType() const {
	return ShortType;
}

SemaType * SymTable::getUIntType() const {
	return UIntType;
}

SemaType * SymTable::getIntType() const {
	return IntType;
}

SemaType * SymTable::getULongType() const {
	return ULongType;
}

SemaType * SymTable::getLongType() const {
	return LongType;
}

SemaType * SymTable::getFloatType() const {
	return FloatType;
}

SemaType * SymTable::getDoubleType() const {
	return DoubleType;
}

SemaType * SymTable::getVoidType() const {
	return VoidType;
}

SemaType * SymTable::getStringType() const {
	return StringType;
}

SemaType * SymTable::getErrorType() const {
	return ErrorType;
}

