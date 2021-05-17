//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGenModule.h - LLVM Module code generation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
/// \file
/// Defines the fly::CodeGenModule interface.
///
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_CODEGENMODULE_H
#define FLY_CODEGENMODULE_H

#include "CharUnits.h"
#include "CodeGenTypeCache.h"
#include "AST/ASTContext.h"
#include "Basic/Diagnostic.h"
#include "Basic/TargetInfo.h"
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>

using namespace llvm;

namespace fly {

    class CodeGenModule : public CodeGenTypeCache {

    private:
        DiagnosticsEngine &Diags;
        ASTNode &AST;
        TargetInfo &Target;
        llvm::LLVMContext VMContext;

    public:
        CodeGenModule(DiagnosticsEngine &Diags, ASTNode &AST, TargetInfo &Target);

        std::unique_ptr<llvm::Module> Module;

        void Generate();

        void GenerateGlobalVar(GlobalVarDecl *Var);

        GlobalVariable* GenerateAndGetGlobalVar(GlobalVarDecl* Var);

        void GenerateGlobalVars(std::vector<GlobalVarDecl*>  Vars);
    };
}

#endif //FLY_CODEGENMODULE_H
