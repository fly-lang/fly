//===-------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenModule.cpp - Emit LLVM Code from ASTs for a Module
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
/// \file
/// Defines the fly::CodeGenModule builder.
/// This builds an AST and converts it to LLVM Code.
///
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenModule.h"

using namespace llvm;
using namespace fly;

CodeGenModule::CodeGenModule(fly::DiagnosticsEngine &diags, llvm::StringRef moduleName) :
                                    diags(diags),
                                    llvmContext(new LLVMContext()),
                                    module(new llvm::Module(moduleName, *llvmContext)) {
}

void CodeGenModule::initialize(TargetInfo &targetInfo) {
    module->setTargetTriple(targetInfo.getTriple().getTriple());
    module->setDataLayout(targetInfo.getDataLayout());
    // module->setSDKVersion() TODO
}

