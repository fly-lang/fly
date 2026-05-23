//===--------------------------------------------------------------------------------------------------------------===//
// compiler/CodeGen/CodeGenEnumEntry.cpp - enum entry code generation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenEnumEntry.h"

#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "Sema/SemaEnumEntry.h"

#include <AST/ASTEnum.h>
#include <Basic/Debug.h>
#include <Sema/SemaEnumType.h>
#include <Sema/SemaModule.h>
#include <llvm/IR/Constants.h>

using namespace fly;

CodeGenEnumEntry::CodeGenEnumEntry(CodeGenModule *CGM, SemaEnumEntry *Sema) : CodeGenExpr(CGM), T(CodeGen::Int32Ty),
        Value(llvm::ConstantInt::get(CodeGen::Int32Ty, Sema->getIndex())) {

}

llvm::Type *CodeGenEnumEntry::getType() {
    return T;
}

llvm::Value *CodeGenEnumEntry::getValue() {
    return Value;
}

size_t CodeGenEnumEntry::getIndex() {
	return Index;
}