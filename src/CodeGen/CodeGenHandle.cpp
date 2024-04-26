//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenClassVar.cpp - Code Generator Class Var
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenHandle.h"
#include "CodeGen/CodeGenModule.h"
#include "llvm/IR/BasicBlock.h"

using namespace fly;

CodeGenHandle::CodeGenHandle(CodeGenModule *CGM) : CGM(CGM) {

}

llvm::BasicBlock *CodeGenHandle::GenBlock() {
    HandleBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "handle");
    return HandleBB;
}

void CodeGenHandle::GoToBlock() {
    CGM->Builder->CreateBr(HandleBB);
}

