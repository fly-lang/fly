//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaBuiltin.cpp - Builtin Types implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaBuiltin.h"
#include "Sema/SemaType.h"

using namespace fly;


SemaType * SemaBuiltin::getBoolType() {
	if (BoolType == nullptr) {
		BoolType = new SemaType(SemaKind::BUILTIN_TYPE, SemaTypeKind::TYPE_BOOL, "bool");
	}
	return BoolType;
}

SemaIntType * SemaBuiltin::getByteType() {
	if (ByteType == nullptr) {
		ByteType = new SemaIntType(SemaIntTypeKind::TYPE_BYTE, "byte");
	}
	return ByteType;
}

SemaIntType * SemaBuiltin::getUShortType() {
	if (UShortType == nullptr) {
		UShortType = new SemaIntType(SemaIntTypeKind::TYPE_USHORT, "ushort");
	}
	return UShortType;
}

SemaIntType * SemaBuiltin::getShortType() {
	if (ShortType == nullptr) {
		ShortType = new SemaIntType(SemaIntTypeKind::TYPE_SHORT, "short");
	}
	return ShortType;
}

SemaIntType * SemaBuiltin::getUIntType() {
	if (UIntType == nullptr) {
		UIntType = new SemaIntType(SemaIntTypeKind::TYPE_UINT, "uint");
	}
	return UIntType;
}

SemaIntType * SemaBuiltin::getIntType() {
	if (IntType == nullptr) {
		IntType = new SemaIntType(SemaIntTypeKind::TYPE_INT, "int");
	}
	return IntType;
}

SemaIntType * SemaBuiltin::getULongType() {
	if (ULongType == nullptr) {
		ULongType = new SemaIntType(SemaIntTypeKind::TYPE_ULONG, "ulong");
	}
	return ULongType;
}

SemaIntType * SemaBuiltin::getLongType() {
	if (LongType == nullptr) {
		LongType = new SemaIntType(SemaIntTypeKind::TYPE_LONG, "long");
	}
	return LongType;
}

SemaFloatType * SemaBuiltin::getFloatType() {
	if (FloatType == nullptr) {
		FloatType = new SemaFloatType(SemaFloatTypeKind::TYPE_FLOAT, "float");
	}
	return FloatType;
}

SemaFloatType * SemaBuiltin::getDoubleType() {
	if (DoubleType == nullptr) {
		DoubleType = new SemaFloatType(SemaFloatTypeKind::TYPE_DOUBLE, "double");
	}
	return DoubleType;
}

SemaType * SemaBuiltin::getVoidType() {
	if (VoidType == nullptr) {
		VoidType = new SemaType(SemaKind::BUILTIN_TYPE, SemaTypeKind::TYPE_VOID, "void");
	}
	return VoidType;
}

SemaType * SemaBuiltin::getStringType() {
	if (StringType == nullptr) {
		StringType = new SemaType(SemaKind::BUILTIN_TYPE, SemaTypeKind::TYPE_STRING, "string");
	}
	return StringType;
}

SemaErrorType * SemaBuiltin::getErrorType() {
	if (ErrorType == nullptr) {
		ErrorType = new SemaErrorType();
	}
	return ErrorType;
}

SemaArrayType * SemaBuiltin::CreateArrayType(SemaType *Type, ASTExpr *SizeExpr) {
	return new SemaArrayType(Type, SizeExpr);
}
