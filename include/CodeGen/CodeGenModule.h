//===---------------------------------------------------------------------===//
// include/CodeGen/CodeGenModule.h - LLVM Module code generation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//
/// \file
/// Defines the fly::CodeGenModule interface.
///
//===----------------------------------------------------------------------===//

#ifndef FLY_CODEGENMODULE_H
#define FLY_CODEGENMODULE_H

#include "AST/ASTContext.h"
#include "Basic/Diagnostic.h"
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <Basic/TargetInfo.h>

using namespace llvm;

namespace fly {

    class CodeGenModule {

    private:
        DiagnosticsEngine &diags;
        std::unique_ptr<llvm::LLVMContext> llvmContext;


    public:
        CodeGenModule(DiagnosticsEngine &diags, llvm::StringRef moduleName);

        void initialize(TargetInfo &targetInfo);

        std::unique_ptr<llvm::Module> module;
    };
}

#endif //FLY_CODEGENMODULE_H
