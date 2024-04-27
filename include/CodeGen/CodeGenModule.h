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
    class ASTCall;
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
    class CodeGenFunctionBase;
    class CodeGenClass;
    class CodeGenVarBase;
    class CodeGenEnum;
    class CodeGenError;
    class ASTClass;
    class ASTVar;
    class ASTEnum;
    class ASTIdentifier;

    class CodeGenModule : public CodeGenTypeCache {

        friend class CodeGenGlobalVar;
        friend class CodeGenFunction;
        friend class CodeGenFunctionBase;
        friend class CodeGenClass;
        friend class CodeGenClassVar;
        friend class CodeGenClassFunction;
        friend class CodeGenVarBase;
        friend class CodeGenInstance;
        friend class CodeGenVar;
        friend class CodeGenExpr;
        friend class CodeGenError;
        friend class CodeGenHandle;

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

        CodeGenEnum *GenEnum(ASTEnum *Enum, bool isExternal = false);

        llvm::Type *GenType(const ASTType *Type);

        llvm::ArrayType *GenArrayType(const ASTArrayType *Type);

        llvm::Constant *GenDefaultValue(const ASTType *Type, llvm::Type *Ty = nullptr);

        llvm::Constant *GenValue(const ASTType *Type, const ASTValue *Val);

        void GenStmt(CodeGenFunctionBase *CGF, ASTStmt * Stmt);

        CodeGenError *GenErrorHandler(ASTVar* Var);

        CodeGenVarBase *GenVar(ASTVar* Var);

        llvm::Value *GenVarRef(ASTVarRef *VarRef);

        llvm::Value *GenCall(ASTCall *Call);

        llvm::Value *GenExpr(ASTExpr *Expr);

        void GenBlock(CodeGenFunctionBase *CGF, const llvm::SmallVector<ASTStmt *, 8> &Content, llvm::BasicBlock *BB = nullptr);

        void GenIfBlock(CodeGenFunctionBase *CGF, ASTIfBlock *If);

        llvm::BasicBlock *GenElsifBlock(CodeGenFunctionBase *CGF,
                                        llvm::BasicBlock *ElsifBB,
                                        llvm::SmallVector<ASTElsifBlock *, 8>::iterator &It);

        void GenSwitchBlock(CodeGenFunctionBase *CGF, ASTSwitchBlock *Switch);

        void GenForBlock(CodeGenFunctionBase *CGF, ASTForBlock *For);

        void GenWhileBlock(CodeGenFunctionBase *CGF, ASTWhileBlock *While);

        void pushArgs(ASTCall *pCall, llvm::SmallVector<llvm::Value *, 8> &Args);

        void GenReturn(ASTFunctionBase *CGF, llvm::Value *V = nullptr);
    };
}

#endif //FLY_CODEGENMODULE_H
