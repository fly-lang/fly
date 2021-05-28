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
        const CodeGenOptions &CodeGenOpts;
        TargetOptions &TargetOpts;
        const ASTContext &Context;
        IntrusiveRefCntPtr<TargetInfo> Target;
        std::unique_ptr<CodeGenModule> Builder;
        BackendAction ActionKind;

        static std::string getOutputFileName(BackendAction Action, StringRef BaseInput);
    public:
        CodeGen(DiagnosticsEngine &Diags, CodeGenOptions &CodeGenOpts, const std::shared_ptr<TargetOptions> &TargetOpts,
                ASTContext &Context, BackendAction Action);

        bool Execute();

        std::unique_ptr<llvm::Module>& getModule();

        static TargetInfo* CreateTargetInfo(DiagnosticsEngine &Diags,
                                                 const std::shared_ptr<TargetOptions> &TargetOpts);

        /// Get the current target info.
        TargetInfo &getTargetInfo() const;

        void GenerateModule(ASTNameSpace * NS);
    };
}

#endif //FLY_CODEGEN_H
