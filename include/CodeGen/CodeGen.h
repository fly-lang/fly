//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGen.h - Code Generator
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
/// \file
/// Defines the fly::CodeGen interface.
///
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_CODEGEN_H
#define FLY_CODEGEN_H

#include "CodeGenModule.h"
#include "CodeGen/BackendUtil.h"
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
        DiagnosticsEngine &Diags;
        std::string ModuleName;
        const CodeGenOptions &CodeGenOpts;
        const TargetOptions &TargetOpts;
        const ASTContext &Context;
        TargetInfo &Target;
        std::unique_ptr<CodeGenModule> Builder;
        BackendAction ActionKind;

        std::unique_ptr<llvm::raw_fd_ostream> getOutputStream(std::error_code &Code);
        static std::string getModuleName(BackendAction Action, StringRef BaseInput);
    public:
        CodeGen(DiagnosticsEngine &Diags, CodeGenOptions &CodeGenOpts, TargetOptions &TargetOpts,
                         ASTContext &Context, TargetInfo &Target, BackendAction Action);

        bool execute();

        std::unique_ptr<llvm::Module>& getModule();
    };
}

#endif //FLY_CODEGEN_H
