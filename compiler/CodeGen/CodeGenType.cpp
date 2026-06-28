//===--------------------------------------------------------------------------------------------------------------===//
// compiler/CodeGen/CodeGenType.cpp - type code generation
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
#include "Sema/SemaEnumType.h"
#include "Sema/SemaType.h"
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
			// ptrsize is pointer-sized. The reference only emits for the host
			// (x86_64), where the pointer width is 64 — use the always-initialised
			// Int64Ty (IntPtrTy can be null at type-gen time). The target-variable
			// lowering lives in the self-host compiler.
			case SemaIntTypeKind::TYPE_PTRSIZE:
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

void CodeGenType::GenType(SemaComplexType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		T = CodeGen::ComplexTy;
	}
}

void CodeGenType::GenType(SemaArrayType &Sema) {
	if (Sema.getCodeGen() == nullptr) {

		// Get the element type of the array
		Sema.getElementType()->accept(*CGM);
		if (Sema.getSizeExpr()) {
			Sema.getSizeExpr()->accept(*CGM);
		}
		T = CodeGen::ArrayTy;
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
		T = CodeGen::StringTy;
	}
}

void CodeGenType::GenType(SemaEnumType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		// Enums are represented as i32 integers
		T = CodeGen::Int32Ty;
	}
}

