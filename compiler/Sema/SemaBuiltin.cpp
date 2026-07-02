//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaBuiltin.cpp - builtin type semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaBuiltin.h"
#include "Sema/SemaType.h"

using namespace fly;

SemaType *SemaBuiltin::BoolType = nullptr;
SemaType * SemaBuiltin::getBoolType() {
	if (BoolType == nullptr) {
		BoolType = new SemaBoolType();
	}
	return BoolType;
}

SemaIntType *SemaBuiltin::ByteType = nullptr;
SemaIntType * SemaBuiltin::getByteType() {
	if (ByteType == nullptr) {
		ByteType = new SemaIntType(SemaIntTypeKind::TYPE_BYTE, "byte");
	}
	return ByteType;
}

SemaIntType *SemaBuiltin::UShortType = nullptr;
SemaIntType * SemaBuiltin::getUShortType() {
	if (UShortType == nullptr) {
		UShortType = new SemaIntType(SemaIntTypeKind::TYPE_USHORT, "ushort");
	}
	return UShortType;
}

SemaIntType *SemaBuiltin::ShortType = nullptr;
SemaIntType * SemaBuiltin::getShortType() {
	if (ShortType == nullptr) {
		ShortType = new SemaIntType(SemaIntTypeKind::TYPE_SHORT, "short");
	}
	return ShortType;
}

SemaIntType *SemaBuiltin::UIntType = nullptr;
SemaIntType * SemaBuiltin::getUIntType() {
	if (UIntType == nullptr) {
		UIntType = new SemaIntType(SemaIntTypeKind::TYPE_UINT, "uint");
	}
	return UIntType;
}

SemaIntType *SemaBuiltin::IntType = nullptr;
SemaIntType * SemaBuiltin::getIntType() {
	if (IntType == nullptr) {
		IntType = new SemaIntType(SemaIntTypeKind::TYPE_INT, "int");
	}
	return IntType;
}

SemaIntType *SemaBuiltin::ULongType = nullptr;
SemaIntType * SemaBuiltin::getULongType() {
	if (ULongType == nullptr) {
		ULongType = new SemaIntType(SemaIntTypeKind::TYPE_ULONG, "ulong");
	}
	return ULongType;
}

SemaIntType *SemaBuiltin::LongType = nullptr;
SemaIntType * SemaBuiltin::getLongType() {
	if (LongType == nullptr) {
		LongType = new SemaIntType(SemaIntTypeKind::TYPE_LONG, "long");
	}
	return LongType;
}

SemaIntType *SemaBuiltin::PtrSizeType = nullptr;
SemaIntType * SemaBuiltin::getPtrSizeType() {
	if (PtrSizeType == nullptr) {
		PtrSizeType = new SemaIntType(SemaIntTypeKind::TYPE_POINTER, "pointer");
	}
	return PtrSizeType;
}

SemaFloatType *SemaBuiltin::FloatType = nullptr;
SemaFloatType * SemaBuiltin::getFloatType() {
	if (FloatType == nullptr) {
		FloatType = new SemaFloatType(SemaFloatTypeKind::TYPE_FLOAT, "float");
	}
	return FloatType;
}

SemaFloatType *SemaBuiltin::DoubleType = nullptr;
SemaFloatType * SemaBuiltin::getDoubleType() {
	if (DoubleType == nullptr) {
		DoubleType = new SemaFloatType(SemaFloatTypeKind::TYPE_DOUBLE, "double");
	}
	return DoubleType;
}

SemaComplexType *SemaBuiltin::ComplexType = nullptr;
SemaComplexType * SemaBuiltin::getComplexType() {
	if (ComplexType == nullptr) {
		ComplexType = new SemaComplexType();
	}
	return ComplexType;
}

SemaType *SemaBuiltin::VoidType = nullptr;
SemaType * SemaBuiltin::getVoidType() {
	if (VoidType == nullptr) {
		VoidType = new SemaVoidType();
	}
	return VoidType;
}

SemaType *SemaBuiltin::StringType = nullptr;
SemaType * SemaBuiltin::getStringType() {
	if (StringType == nullptr) {
		StringType = new SemaStringType();
	}
	return StringType;
}

SemaErrorType *SemaBuiltin::ErrorType = nullptr;
SemaErrorType * SemaBuiltin::getErrorType() {
	if (ErrorType == nullptr) {
		ErrorType = new SemaErrorType();
	}
	return ErrorType;
}

SemaArrayType * SemaBuiltin::CreateArrayType(SemaType *Type, SemaExpr *SizeExpr) {
	return new SemaArrayType(Type, SizeExpr);
}

SemaArrayType * SemaBuiltin::CreateArrayType(SemaType *Type, uint64_t Size) {
	return new SemaArrayType(Type, Size);
}

void SemaBuiltin::resetCodeGen() {
	if (BoolType) BoolType->setCodeGen(nullptr);
	if (ByteType) ByteType->setCodeGen(nullptr);
	if (UShortType) UShortType->setCodeGen(nullptr);
	if (ShortType) ShortType->setCodeGen(nullptr);
	if (UIntType) UIntType->setCodeGen(nullptr);
	if (IntType) IntType->setCodeGen(nullptr);
	if (ULongType) ULongType->setCodeGen(nullptr);
	if (LongType) LongType->setCodeGen(nullptr);
	if (PtrSizeType) PtrSizeType->setCodeGen(nullptr);
	if (FloatType) FloatType->setCodeGen(nullptr);
	if (DoubleType) DoubleType->setCodeGen(nullptr);
	if (ComplexType) ComplexType->setCodeGen(nullptr);
	if (VoidType) VoidType->setCodeGen(nullptr);
	if (StringType) StringType->setCodeGen(nullptr);
	if (ErrorType) ErrorType->setCodeGen(nullptr);
}

