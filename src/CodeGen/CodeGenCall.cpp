//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenCall.cpp - Code Generator Function implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenCall.h"
#include "CodeGen/CodeGen.h"
#include "llvm/IR/DerivedTypes.h"

using namespace fly;

CodeGenCall::CodeGenCall(CodeGenModule *CGM, llvm::Function *Fn, const llvm::ArrayRef<llvm::Value *> &Args) :
    CGM(CGM), Fn(Fn), Args(Args) {

}

llvm::CallInst *CodeGenCall::Call() const {
    return CGM->Builder->CreateCall(Fn, Args);
}
