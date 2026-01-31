//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGExpr.cpp - Code Generator Expression implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenType.h"

#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "AST/ASTExpr.h"
#include "AST/ASTValue.h"
#include "Sema/SemaValue.h"

using namespace fly;

CodeGenType::CodeGenType(CodeGenModule *CGM) : CodeGenBase(), CGM(CGM) {

}

llvm::Type *CodeGenType::getType() {
	return T;
}

void CodeGenType::GenType(SemaBoolType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		T = CodeGen::BoolTy;
	}
}

void CodeGenType::GenType(SemaIntType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		SemaIntTypeKind IntKind = Sema.getIntKind();
		switch (IntKind) {
			case SemaIntTypeKind::TYPE_BYTE:
				T = CodeGen::Int8Ty;
				break;
			case SemaIntTypeKind::TYPE_USHORT:
			case SemaIntTypeKind::TYPE_SHORT:
				T = CodeGen::Int16Ty;
				break;
			case SemaIntTypeKind::TYPE_UINT:
			case SemaIntTypeKind::TYPE_INT:
				T = CodeGen::Int32Ty;
				break;
			case SemaIntTypeKind::TYPE_ULONG:
			case SemaIntTypeKind::TYPE_LONG:
				T = CodeGen::Int64Ty;
				break;
		}
	}
}

void CodeGenType::GenType(SemaFloatType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		SemaFloatTypeKind FPKind = Sema.getFloatKind();
		switch (FPKind) {
			case SemaFloatTypeKind::TYPE_FLOAT:
				T = CodeGen::FloatTy;
				break;
			case SemaFloatTypeKind::TYPE_DOUBLE:
				T = CodeGen::DoubleTy;
				break;
		}
	}
}

void CodeGenType::GenType(SemaArrayType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		// Get the element type of the array
		Sema.getElementType()->accept(*CGM);
		llvm::Type *ElementType = Sema.getElementType()->getCodeGen()->getType();

		// Check if the array has a size (fixed-size array vs dynamic array)
		SemaExpr *SizeExpr = Sema.getSizeExpr();
		if (SizeExpr) {
			SizeExpr->accept(*CGM);
			llvm::Value *SizeValue = SizeExpr->getCodeGen()->getValue();

			// Cast to ConstantInt to extract the uint64_t value
			if (llvm::ConstantInt *CI = llvm::dyn_cast<llvm::ConstantInt>(SizeValue)) {
				uint64_t ArraySize = CI->getZExtValue();
				T = llvm::ArrayType::get(ElementType, ArraySize);
			}
		}

		// If no SizeExpr, use the Size stored in SemaArrayType (fixed-size array)
		T = llvm::ArrayType::get(ElementType, Sema.getSize());
	}
}

void CodeGenType::GenType(SemaErrorType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		T = CodeGen::ErrorTy;
	}
}

void CodeGenType::GenType(SemaVoidType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		T = CodeGen::VoidTy;
	}
}

void CodeGenType::GenType(SemaStringType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		T = CodeGen::Int8PtrTy;
	}
}
