//===--------------------------------------------------------------------------------------------------------------===//
// compiler/CodeGen/CodeGenHandle.cpp - handle statement code generation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenHandle.h"

#include "CodeGen/CodeGenFunctionBase.h"
#include "CodeGen/CodeGenModule.h"

#include "llvm/IR/BasicBlock.h"

using namespace fly;

CodeGenHandle::CodeGenHandle(CodeGenModule *CGM, CodeGenFunctionBase *CGF) : CGM(CGM) {
	HandleBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "handle", CGF->getFunction());
	SafeBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "safe", CGF->getFunction());
}

llvm::BasicBlock *CodeGenHandle::getHandleBlock() {
    return HandleBB;
}

llvm::BasicBlock *CodeGenHandle::getSafeBlock() {
	return SafeBB;
}
