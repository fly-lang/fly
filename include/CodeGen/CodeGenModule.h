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
#include "Basic/Diagnostic.h"
#include "Basic/TargetInfo.h"
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <AST/ASTBlockStmt.h>
#include <AST/ASTIfStmt.h>

using namespace llvm;

namespace fly {

    class ASTContext;
    class ASTModule;
    class ASTImport;
    class ASTGlobalVar;
    class ASTFunction;
    class ASTCall;
    class ASTType;
    class ASTArrayType;
    class ASTValue;
    class ASTStmt;
    class ASTExpr;
    class ASTOpExpr;
    class ASTIfStmt;
    class ASTSwitchStmt;
    class ASTLoopStmt;
    class ASTClass;
    class ASTVar;
    class ASTEnum;
    class ASTIdentifier;
    class ASTVarRef;
    class CodeGen;
    class CodeGenGlobalVar;
    class CodeGenFunction;
    class CodeGenFunctionBase;
    class CodeGenClass;
    class CodeGenVar;
    class CodeGenEnum;
    class CodeGenError;

    class CodeGenModule {

        friend class CodeGen;
        friend class CodeGenGlobalVar;
        friend class CodeGenFunction;
        friend class CodeGenFunctionBase;
        friend class CodeGenClass;
        friend class CodeGenClassVar;
        friend class CodeGenClassFunction;
        friend class CodeGenVarBase;
        friend class CodeGenVar;
        friend class CodeGenExpr;
        friend class CodeGenError;
        friend class CodeGenHandle;

        // Diagnostics
        DiagnosticsEngine &Diags;

        // CodeGen Options
        CodeGenOptions &CGOpts;

        ASTModule &AST;

        // Target Info
        TargetInfo &Target;

        // LLVM Context
        llvm::LLVMContext &LLVMCtx;

        // LLVM Builder
        llvm::IRBuilder<> *Builder;

        // LLVM Module
        llvm::Module *Module;

        // CGDebugInfo *DebugInfo; // TODO

    public:

        /// void
        llvm::Type *VoidTy;

        /// i8, i16, i32, and i64
        llvm::IntegerType *BoolTy, *Int8Ty, *Int16Ty, *Int32Ty, *Int64Ty;
        /// half, bfloat, float, double
        llvm::Type *HalfTy, *BFloatTy, *FloatTy, *DoubleTy;

        /// int
        llvm::IntegerType *IntTy;

        /// intptr_t, size_t, and ptrdiff_t, which we assume are the same size.
        union {
            llvm::IntegerType *IntPtrTy;
            llvm::IntegerType *SizeTy;
            llvm::IntegerType *PtrDiffTy;
        };

        /// void* in address space 0
        union {
            llvm::PointerType *VoidPtrTy;
            llvm::PointerType *Int8PtrTy;
        };

        /// void** in address space 0
        union {
            llvm::PointerType *VoidPtrPtrTy;
            llvm::PointerType *Int8PtrPtrTy;
        };

        /// void* in alloca address space
        union {
            llvm::PointerType *AllocaVoidPtrTy;
            llvm::PointerType *AllocaInt8PtrTy;
        };

        /// The width of a pointer into the generic address space.
        unsigned char PointerWidthInBits;

        /// The size and alignment of a pointer into the generic address space.
        union {
            unsigned char PointerAlignInBytes;
            unsigned char PointerSizeInBytes;
        };

        /// The size and alignment of size_t.
        union {
            unsigned char SizeSizeInBytes; // sizeof(size_t)
            unsigned char SizeAlignInBytes;
        };

        /// The size and alignment of the builtin C type 'int'.  This comes
        /// up enough in various ABI lowering tasks to be worth pre-computing.
        union {
            unsigned char IntSizeInBytes;
            unsigned char IntAlignInBytes;
        };

        llvm::StructType *ErrorTy;

        llvm::PointerType *ErrorPtrTy;

        CodeGenModule(DiagnosticsEngine &Diags, ASTModule &AST, LLVMContext &LLVMCtx, TargetInfo &Target,
                      CodeGenOptions &CGOpts);

        virtual ~CodeGenModule();

        ASTModule &getAst() const;

        DiagnosticBuilder Diag(const SourceLocation &Loc, unsigned DiagID);

        llvm::Module *getModule() const;

        void GenAll();

        void GenHeaders();

        CodeGenGlobalVar *GenGlobalVar(ASTGlobalVar *GlobalVar, bool isExternal = false);

        CodeGenFunction *GenFunction(ASTFunction *Function, bool isExternal = false);

        CodeGenClass *GenClass(ASTClass *Class, bool isExternal = false);

        void GenEnum(ASTEnum *Enum);

        llvm::Type *GenType(const ASTType *Type);

        llvm::ArrayType *GenArrayType(const ASTArrayType *Type);

        llvm::Constant *GenDefaultValue(const ASTType *Type, llvm::Type *Ty = nullptr);

        llvm::Constant *GenValue(const ASTType *Type, const ASTValue *Val);

//        llvm::Value *Convert(llvm::Value *V, llvm::Type *T);

        llvm::Value *ConvertToBool(llvm::Value *Val);

        llvm::Value *Convert(llvm::Value *FromVal, const ASTType *FromType, const ASTType *ToType);

        CodeGenError *GenErrorHandler(ASTVar* Var);

        CodeGenVar *GenLocalVar(ASTLocalVar* Var);

        llvm::Value *GenVarRef(ASTVarRef *VarRef);

        llvm::Value *GenCall(ASTCall *Call);

        llvm::Value *GenExpr(ASTExpr *Expr);

        void GenStmt(CodeGenFunctionBase *CGF, ASTStmt * Stmt);

        void GenBlock(CodeGenFunctionBase *CGF, ASTBlockStmt *BlockStmt);

        void GenIfBlock(CodeGenFunctionBase *CGF, ASTIfStmt *If);

        llvm::BasicBlock *GenElsifBlock(CodeGenFunctionBase *CGF,
                                        llvm::BasicBlock *ElsifBB,
                                        llvm::SmallVector<ASTElsif *, 8>::iterator &It);

        void GenSwitchBlock(CodeGenFunctionBase *CGF, ASTSwitchStmt *Switch);

        void GenLoopBlock(CodeGenFunctionBase *CGF, ASTLoopStmt *Loop);

        void GenReturn(ASTFunctionBase *CGF, ASTExpr *Expr = nullptr);
    };
}

#endif //FLY_CODEGENMODULE_H
