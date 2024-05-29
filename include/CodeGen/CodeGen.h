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

#include "CodeGen/BackendUtil.h"
#include "Basic/Diagnostic.h"
#include "AST/ASTContext.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Linker/Linker.h"
#include <memory>

namespace llvm {
    class Constant;
    class LLVMContext;
    class Module;
    class StringRef;
}

namespace fly {

    class CodeGenHeader;
    class CodeGenModule;
    class TargetInfo;
    class FrontendOptions;

    class CodeGen {

    protected:
        DiagnosticsEngine &Diags;
        CodeGenOptions &CodeGenOpts;
        TargetOptions &TargetOpts;
        IntrusiveRefCntPtr<TargetInfo> Target;
        llvm::LLVMContext LLVMCtx;
        BackendActionKind ActionKind;
        bool ShowTimers;

    public:
        CodeGen(DiagnosticsEngine &Diags, CodeGenOptions &CodeGenOpts,
                const std::shared_ptr<TargetOptions> &TargetOpts,
                BackendActionKind BackendAction, bool ShowTimers = false);

        std::string getOutputFileName(StringRef BaseInput);

        void Emit(llvm::Module *M, llvm::StringRef OutName);

        static TargetInfo* CreateTargetInfo(DiagnosticsEngine &Diags,
                                                 const std::shared_ptr<TargetOptions> &TargetOpts);

        /// Get the current target info.
        TargetInfo &getTargetInfo() const;

        llvm::LLVMContext &getLLVMCtx();

        std::vector<llvm::Module *> GenerateModules(ASTContext &Context);

        CodeGenModule *GenerateModule(ASTModule &AST);

        void GenerateHeaders(ASTContext &context);

        void GenerateHeader(ASTModule &Module);

        static std::string toIdentifier(llvm::StringRef Name, llvm::StringRef NameSpace, llvm::StringRef ClassName = "");
    };
}

#endif //FLY_CODEGEN_H
