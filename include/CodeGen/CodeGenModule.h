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
#include <llvm/IR/IRBuilder.h>

using namespace llvm;

namespace fly {

    class CodeGenModule : public CodeGenTypeCache {

        friend class CGGlobalVar;
        friend class CGFunction;
        friend class CGVar;

    private:
        DiagnosticsEngine &Diags;
        ASTNode &Node;
        TargetInfo &Target;
        llvm::LLVMContext VMContext;
        llvm::IRBuilder<> *Builder;

    public:
        CodeGenModule(DiagnosticsEngine &Diags, ASTNode &Node, TargetInfo &Target);

        virtual ~CodeGenModule();

        std::unique_ptr<llvm::Module> Module;

        void Generate();

        CGGlobalVar *GenGlobalVar(GlobalVarDecl *VDecl);

        CGFunction *GenFunction(FuncDecl *FDecl);

        Type *GenTypeValue(const TypeBase *TyData, StringRef StrVal = "", llvm::Constant *InitVal = nullptr);

        void GenStmt(Stmt * S);

        void GenExpr(const TypeBase *Typ, GroupExpr *pExpr);
    };
}

#endif //FLY_CODEGENMODULE_H
