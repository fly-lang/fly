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
#include <AST/ASTBlock.h>
#include <AST/ASTIfBlock.h>

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
    class ASTGroupExpr;
    class ASTIfBlock;
    class ASTSwitchBlock;
    class ASTForBlock;
    class ASTWhileBlock;

    class CodeGenModule : public CodeGenTypeCache {

        friend class CodeGenGlobalVar;
        friend class CodeGenFunction;
        friend class CodeGenCall;
        friend class CodeGenLocalVar;
        friend class CodeGenExpr;

    private:
        DiagnosticsEngine &Diags;
        CodeGenOptions &CGOpts;
        ASTNode &Node;
        TargetInfo &Target;
        llvm::LLVMContext &LLVMCtx;
        llvm::IRBuilder<> *Builder;
        // CGDebugInfo *DebugInfo; // TODO

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

        Type *GenType(const ASTType *Ty);

        llvm::Constant *GenValue(const ASTType *TyData, const ASTValue *Val);

        void GenStmt(ASTStmt * S);

        llvm::Value *GenExpr(const ASTType *Typ, ASTExpr *Expr);

        void GenBlock(const std::vector<ASTStmt *> &Content, llvm::BasicBlock *BB = nullptr);

        void GenIfBlock(ASTIfBlock *If);

        llvm::BasicBlock *GenElsifBlock(llvm::Value *Cond, llvm::BasicBlock *TrueBB, ASTIfBlock *TrueBlock,
                                        std::vector<ASTElsifBlock *>::iterator It);

        void GenSwitchBlock(ASTSwitchBlock *Switch);

        void GenForBlock(ASTForBlock *For);

        void GenWhileBlock(ASTWhileBlock *While);
    };
}

#endif //FLY_CODEGENMODULE_H
