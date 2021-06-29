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
    class GlobalVarDecl;
    class FuncDecl;
    class FuncCall;
    class TypeBase;
    class CodeGenGlobalVar;
    class CodeGenFunction;
    class CodeGenCall;
    class Stmt;
    class GroupExpr;

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

        void GenTypeValue(const TypeBase *TyData, llvm::Type *&Ty, llvm::Constant *&Const, StringRef StrVal);

    public:
        CodeGenModule(DiagnosticsEngine &Diags, ASTNode &Node, LLVMContext &LLVMCtx, TargetInfo &Target,
                      CodeGenOptions &CGOpts);

        virtual ~CodeGenModule();

        llvm::Module *Module;

        void Generate();

        CodeGenGlobalVar *GenGlobalVar(GlobalVarDecl *VDecl);

        CodeGenFunction *GenFunction(FuncDecl *FDecl);

        CallInst *GenCall(FuncCall *Call);

        Type *GenType(const TypeBase *TyData);

        llvm::Constant *GenValue(const TypeBase *TyData, StringRef StrVal);

        void GenStmt(Stmt * S);

        llvm::Value *GenExpr(const TypeBase *Typ, GroupExpr *pExpr);
    };
}

#endif //FLY_CODEGENMODULE_H
