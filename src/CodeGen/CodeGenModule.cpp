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

CodeGenModule::CodeGenModule(fly::DiagnosticsEngine &Diags, llvm::StringRef ModuleName) :
                                    Diags(Diags),
                                    VMContext(new LLVMContext()),
                                    Module(new llvm::Module(ModuleName, *VMContext)) {
}

void CodeGenModule::Initialize(TargetInfo &Target) {
    Module->setTargetTriple(Target.getTriple().getTriple());
    Module->setDataLayout(Target.getDataLayout());
    const auto &SDKVersion = Target.getSDKVersion();
    if (!SDKVersion.empty())
        Module->setSDKVersion(SDKVersion);

    // todo Add dependencies, Linker Options
}

