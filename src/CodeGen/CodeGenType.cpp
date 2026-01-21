//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGExpr.cpp - Code Generator Expression implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenType.h"
#include "CodeGen/CodeGenModule.h"

using namespace fly;

CodeGenType::CodeGenType(CodeGenModule *CGM) : CodeGenBase(), CGM(CGM) {

}

llvm::Type *CodeGenType::getType() {
	return T;
}

void CodeGenType::GenType(SemaBoolType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		T = CGM->BoolTy;
	}
}

void CodeGenType::GenType(SemaIntType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		SemaIntTypeKind IntKind = Sema.getIntKind();
		switch (IntKind) {
			case SemaIntTypeKind::TYPE_BYTE:
				T = CGM->Int8Ty;
				break;
			case SemaIntTypeKind::TYPE_USHORT:
			case SemaIntTypeKind::TYPE_SHORT:
				T = CGM->Int16Ty;
				break;
			case SemaIntTypeKind::TYPE_UINT:
			case SemaIntTypeKind::TYPE_INT:
				T = CGM->Int32Ty;
				break;
			case SemaIntTypeKind::TYPE_ULONG:
			case SemaIntTypeKind::TYPE_LONG:
				T = CGM->Int64Ty;
				break;
		}
	}
}

void CodeGenType::GenType(SemaFloatType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		SemaFloatTypeKind FPKind = Sema.getFPKind();
		switch (FPKind) {
			case SemaFloatTypeKind::TYPE_FLOAT:
				T = CGM->FloatTy;
				break;
			case SemaFloatTypeKind::TYPE_DOUBLE:
				T = CGM->DoubleTy;
				break;
		}
	}
}

void CodeGenType::GenType(SemaArrayType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		// Get the element type of the array
		Sema.getType()->accept(*CGM);
		llvm::Type *ElementType = Sema.getType()->getCodeGen()->getType();

		// Arrays are represented as pointers to the element type
		T = llvm::PointerType::getUnqual(ElementType);
	}
}

void CodeGenType::GenType(SemaErrorType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		T = CGM->ErrorTy;
	}
}

void CodeGenType::GenType(SemaVoidType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		T = CGM->VoidTy;
	}
}

void CodeGenType::GenType(SemaStringType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		T = CGM->Int8PtrTy;
	}
}
