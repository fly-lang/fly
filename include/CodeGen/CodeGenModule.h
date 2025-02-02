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

#ifndef FLY_CODEGEN_MODULE_H
#define FLY_CODEGEN_MODULE_H

#include "Basic/Diagnostic.h"
#include "Basic/TargetInfo.h"
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <Sym/SymType.h>

using namespace llvm;

namespace fly {

    class SymNameSpace;
    class SymTable;
    class CodeGen;
    class CodeGenGlobalVar;
    class CodeGenFunction;
    class CodeGenFunctionBase;
    class CodeGenClass;
    class CodeGenVar;
    class CodeGenEnum;
    class CodeGenError;
class ASTCall;
class ASTFailStmt;
class ASTLoopStmt;
class ASTSwitchStmt;
class ASTValue;
class ASTVar;
class ASTRuleStmt;
class ASTVarRef;
class ASTFunction;
class ASTExpr;
class ASTStmt;
class ASTBlockStmt;
class ASTIfStmt;
class SymGlobalVar;
class SymFunction;
class SymClass;
class SymClassAttribute;
class SymClassMethod;
class SymEnum;
class SymFunctionBase;

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

        SymNameSpace &NameSpace;

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

        CodeGenModule(DiagnosticsEngine &Diags, SymNameSpace &NameSpace, LLVMContext &LLVMCtx, TargetInfo &Target,
                      CodeGenOptions &CGOpts);

        virtual ~CodeGenModule();

        SymNameSpace &getNameSpace() const;

        DiagnosticBuilder Diag(const SourceLocation &Loc, unsigned DiagID);

        llvm::Module *getModule() const;

        void GenAll();

        void GenHeaders();

        CodeGenGlobalVar *GenGlobalVar(SymGlobalVar *GlobalVar, bool isExternal = false);

        CodeGenFunction *GenFunction(SymFunction *Function, bool isExternal = false);

        CodeGenClass *GenClass(SymClass *Class, bool isExternal = false);

        void GenEnum(SymEnum *Enum);

        llvm::Type *GenType(const SymType *Type);

        llvm::ArrayType *GenArrayType(const SymTypeArray *Type);

        llvm::Constant *GenDefaultValue(const SymType *Type, llvm::Type *Ty = nullptr);

        llvm::Constant *GenValue(const SymType *Type, const ASTValue *Val);

//        llvm::Value *Convert(llvm::Value *V, llvm::Type *T);

        llvm::Value *ConvertToBool(llvm::Value *Val);

        llvm::Value *Convert(llvm::Value *FromVal, const SymType *FromType, const SymType *ToType);

        CodeGenError *GenErrorHandler(ASTVar* Var);

        CodeGenVar *GenLocalVar(ASTVar* Var);

        llvm::Value *GenVarRef(ASTVarRef *VarRef);

        llvm::Value *GenCall(ASTCall *Call);

        llvm::Value *GenExpr(ASTExpr *Expr);

        void GenFailStmt(ASTFailStmt *FailStmt, CodeGenError *CGH);

        void GenStmt(CodeGenFunctionBase *CGF, ASTStmt * Stmt);

        void GenBlock(CodeGenFunctionBase *CGF, ASTBlockStmt *BlockStmt);

        void GenIfBlock(CodeGenFunctionBase *CGF, ASTIfStmt *If);

        llvm::BasicBlock *GenElsifBlock(CodeGenFunctionBase *CGF,
                                        llvm::BasicBlock *ElsifBB,
                                        llvm::SmallVector<ASTRuleStmt *, 8>::iterator &It);

        void GenSwitchBlock(CodeGenFunctionBase *CGF, ASTSwitchStmt *Switch);

        void GenLoopBlock(CodeGenFunctionBase *CGF, ASTLoopStmt *Loop);

        void GenReturn(ASTFunction *CGF, ASTExpr *Expr = nullptr);
    };
}

#endif //FLY_CODEGEN_MODULE_H
