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
#include "CodeGenHeader.h"
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
    class ASTImport;
    class ASTGlobalVar;
    class ASTFunction;
    class ASTFunctionCall;
    class ASTType;
    class ASTArrayType;
    class ASTValue;
    class ASTStmt;
    class ASTExpr;
    class ASTGroupExpr;
    class ASTIfBlock;
    class ASTSwitchBlock;
    class ASTForBlock;
    class ASTWhileBlock;
    class CodeGenGlobalVar;
    class CodeGenFunction;
    class CodeGenCall;
    class CodeGenClass;
    class ASTClass;

    class CodeGenModule : public CodeGenTypeCache {

        friend class CodeGenGlobalVar;
        friend class CodeGenFunction;
        friend class CodeGenClass;
        friend class CodeGenClassVar;
        friend class CodeGenClassFunction;
        friend class CodeGenCall;
        friend class CodeGenStackVar;
        friend class CodeGenVar;
        friend class CodeGenExpr;

        // Diagnostics
        DiagnosticsEngine &Diags;

        // CodeGen Options
        CodeGenOptions &CGOpts;

        // Target Info
        TargetInfo &Target;

        // LLVM Context
        llvm::LLVMContext &LLVMCtx;

        // LLVM Builder
        llvm::IRBuilder<> *Builder;

        // CGDebugInfo *DebugInfo; // TODO

    public:
        // LLVM Module
        std::unique_ptr<llvm::Module> Module;

        CodeGenModule(DiagnosticsEngine &Diags, llvm::StringRef Name, LLVMContext &LLVMCtx, TargetInfo &Target,
                      CodeGenOptions &CGOpts);

        virtual ~CodeGenModule();

        DiagnosticBuilder Diag(const SourceLocation &Loc, unsigned DiagID);

        llvm::Module *getModule() const;

        llvm::Module *ReleaseModule();

        CodeGenGlobalVar *GenGlobalVar(ASTGlobalVar *GlobalVar, bool isExternal = false);

        CodeGenFunction *GenFunction(ASTFunction *Function, bool isExternal = false);

        CodeGenClass *GenClass(ASTClass *Class, bool isExternal = false);

        llvm::Type *GenType(const ASTType *Type);

        llvm::ArrayType *GenArrayType(const ASTArrayType *Type);

        llvm::Constant *GenDefaultValue(const ASTType *Type, llvm::Type *Ty = nullptr);

        llvm::Constant *GenValue(const ASTType *Type, const ASTValue *Val);

        void GenStmt(llvm::Function *Fn, ASTStmt * Stmt);

        CallInst *GenCall(llvm::Function *Fn, ASTFunctionCall *Call);

        llvm::Value *GenExpr(llvm::Function *Fn, const ASTType *Type, ASTExpr *Expr);

        void GenBlock(llvm::Function *Fn, const std::vector<ASTStmt *> &Content, llvm::BasicBlock *BB = nullptr);

        void GenIfBlock(llvm::Function *Fn, ASTIfBlock *If);

        llvm::BasicBlock *GenElsifBlock(llvm::Function *Fn,
                                        llvm::BasicBlock *ElsifBB,
                                        std::vector<ASTElsifBlock *>::iterator &It);

        void GenSwitchBlock(llvm::Function *Fn, ASTSwitchBlock *Switch);

        void GenForBlock(llvm::Function *Fn, ASTForBlock *For);

        void GenWhileBlock(llvm::Function *Fn, ASTWhileBlock *While);
    };
}

#endif //FLY_CODEGENMODULE_H
