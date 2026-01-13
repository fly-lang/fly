//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenEnumVar.cpp - Code Generator Enum Var
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenEnumValue.h"

#include "CodeGen/CodeGenModule.h"
#include "Sema/SemaEnumValue.h"

#include <AST/ASTEnum.h>
#include <Basic/Debug.h>
#include <Sema/SemaEnumType.h>
#include <Sema/SemaModule.h>
#include <llvm/IR/Constants.h>

using namespace fly;

CodeGenEnumValue::CodeGenEnumValue(CodeGenModule *CGM, SemaEnumValue *Sema) : CGM(CGM), T(CGM->Int32Ty),
        Value(llvm::ConstantInt::get(CGM->Int32Ty, Sema->getIndex())) {

}

llvm::Type *CodeGenEnumValue::getType() {
    return T;
}

llvm::StoreInst *CodeGenEnumValue::Store(llvm::Value *Val) {
    // FIXME give error
    return nullptr;
}

llvm::LoadInst *CodeGenEnumValue::Load() {
    // FIXME give error
    return nullptr;
}

llvm::Value *CodeGenEnumValue::getValue() {
    return Value;
}

llvm::Value * CodeGenEnumValue::getPointer() {
	return nullptr;
}

void CodeGenEnumValue::setPointer(llvm::Value *Pointer) {
}

size_t CodeGenEnumValue::getIndex() {
	return Index;
}