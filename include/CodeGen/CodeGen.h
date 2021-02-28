//===---------------------------------------------------------------------===//
// include/CodeGen/CodeGen.h - Code Generator
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//
/// \file
/// Defines the fly::CodeGen interface.
///
//===----------------------------------------------------------------------===//

#ifndef FLY_CODEGEN_H
#define FLY_CODEGEN_H

#include "CodeGenModule.h"
#include "Basic/Diagnostic.h"
#include "AST/ASTContext.h"
#include <memory>

namespace llvm {
    class Constant;
    class LLVMContext;
    class Module;
    class StringRef;
}

namespace fly {
    class CodeGenModule;

    class CodeGen {

    protected:
        DiagnosticsEngine &diags;
        const ASTContext &context;
        TargetInfo &target;
        std::unique_ptr<CodeGenModule> builder;

    public:
        explicit CodeGen(DiagnosticsEngine &diags, ASTContext &context, TargetInfo &target);

        bool execute() const;


    };
}

#endif //FLY_CODEGEN_H
