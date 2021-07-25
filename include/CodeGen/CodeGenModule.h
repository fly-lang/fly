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
#include "Basic/Diagnostic.h"
#include "Basic/TargetInfo.h"
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>

using namespace llvm;

namespace fly {

    class ASTContext;
    class ASTNode;
    class ASTValue;
    class ASTGlobalVar;
    class ASTFunc;
    class ASTFuncCall;
    class ASTType;
    class CodeGenGlobalVar;
    class CodeGenFunction;
    class CodeGenCall;
    class ASTStmt;
    class ASTExpr;

    class CodeGenModule : public CodeGenTypeCache {

        friend class CodeGenGlobalVar;
        friend class CodeGenFunction;
        friend class CodeGenCall;
        friend class CodeGenVar;

    private:
        DiagnosticsEngine &Diags;
        CodeGenOptions &CGOpts;
        ASTNode &Node;
        TargetInfo &Target;
        llvm::LLVMContext &LLVMCtx;
        llvm::IRBuilder<> *Builder;
        // CGDebugInfo *DebugInfo; // TODO

        void GenTypeValue(const ASTType *TyData, llvm::Type *&Ty, llvm::Constant *&Const, ASTValue *Val);

    public:
        CodeGenModule(DiagnosticsEngine &Diags, ASTNode &Node, LLVMContext &LLVMCtx, TargetInfo &Target,
                      CodeGenOptions &CGOpts);

        virtual ~CodeGenModule();

        llvm::Module *Module;

        DiagnosticBuilder Diag(const SourceLocation &Loc, unsigned DiagID);

        bool Generate();

        CodeGenGlobalVar *GenGlobalVar(ASTGlobalVar *VDecl);

        CodeGenFunction *GenFunction(ASTFunc *FDecl);

        CallInst *GenCall(ASTFuncCall *Call);

        Type *GenType(const ASTType *TyData);

        llvm::Constant *GenValue(const ASTType *TyData, const ASTValue *Val);

        void GenStmt(ASTStmt * S);

        llvm::Value *GenExpr(const ASTType *Typ, ASTExpr *Expr);
    };
}

#endif //FLY_CODEGENMODULE_H
