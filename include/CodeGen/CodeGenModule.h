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
#include "Sema/SemaVisitor.h"
#include <Sema/SemaType.h>
#include <llvm/IR/IRBuilder.h>

namespace llvm {
    class LLVMContext;
    class Module;
    class Type;
    class Value;
    class PointerType;
    class StructType;
    class IntegerType;
    class Constant;
    class BasicBlock;
    class Value;
};

namespace fly {

    class SymbolTable;
    class CodeGen;
    class CodeGenGlobalVar;
    class CodeGenFunction;
    class CodeGenFunctionBase;
    class CodeGenClass;
    class CodeGenVar;
    class CodeGenBase;
    class CodeGenError;
    class ASTCall;
    class ASTFailStmt;
    class ASTLoopStmt;
    class ASTLoopInStmt;
    class ASTSwitchStmt;
    class ASTValue;
    class ASTVar;
    class ASTRuleStmt;
    class ASTIdentifier;
    class ASTFunction;
    class ASTExpr;
    class ASTStmt;
    class ASTBlockStmt;
    class ASTIfStmt;
    class ASTDeclStmt;
    class ASTExprStmt;
    class ASTDeleteStmt;
    class ASTBreakStmt;
    class ASTContinueStmt;
    class ASTReturnStmt;
    class ASTHandleStmt;
    class SemaModule;
    class SemaGlobalVar;
    class SemaFunction;
    class SemaClassType;
    class SemaEnumType;
    class SemaFunctionBase;
    class SemaVar;
    class SemaCall;
    class SemaValue;
    class SemaExpr;
    class SemaNameSpace;
    class SemaClassMethod;
    class SemaClassAttribute;
    class SemaImport;
    class SemaType;
    class SemaIntType;
    class SemaFloatType;
    class SemaArrayType;
    class SemaErrorType;
    class SemaLocalVar;
    class SemaParam;
    class SemaMember;
    class SemaClassInstance;
    class SemaErrorHandler;
    class SemaUnary;
    class SemaBinary;
    class SemaTernary;
    class SemaCast;
    class SemaBoolValue;
    class SemaIntValue;
    class SemaFloatValue;
    class SemaStringValue;
    class SemaArrayValue;
    class SemaStructValue;
    class SemaNullValue;
    class SemaEnumValue;

    class CodeGenModule : public SemaVisitor {

        friend class CodeGen;
        friend class CodeGenFunction;
        friend class CodeGenFunctionBase;
        friend class CodeGenClass;
        friend class CodeGenClassMethod;
        friend class CodeGenHandle;
    	friend class CodeGenVar;
    	friend class CodeGenError;
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

        llvm::ConstantInt *Zero;

    	llvm::SmallVector<SemaFunctionBase *, 8> Functions;

        // Stack for tracking break/continue targets in loops and switches
        llvm::SmallVector<llvm::BasicBlock *, 8> BreakTargetStack;
        llvm::SmallVector<llvm::BasicBlock *, 8> ContinueTargetStack;

        SemaFunctionBase *CurrentFunction;

        CodeGenModule(DiagnosticsEngine &Diags, StringRef Name, llvm::LLVMContext &LLVMCtx, TargetInfo &Target,
                      CodeGenOptions &CGOpts);

        virtual ~CodeGenModule();

        DiagnosticBuilder Diag(unsigned DiagID);

        llvm::Module *getModule() const;

    	llvm::LLVMContext &getLLVMCtx() const;

    	llvm::IRBuilder<> *getBuilder() const;

        // SemaVisitor interface implementation
        void visit(SemaModule &Sema) override;
        void visit(SemaNameSpace &Sema) override;
        void visit(SemaImport &Sema) override;

        // Types
        void visit(SemaBoolType &Sema) override;
        void visit(SemaIntType &Sema) override;
        void visit(SemaFloatType &Sema) override;
        void visit(SemaArrayType &Sema) override;
        void visit(SemaErrorType &Sema) override;
    	void visit(SemaVoidType &Sema) override;
    	void visit(SemaStringType &Sema) override;
    	void visit(SemaEnumType &Sema) override;
    	void visit(SemaClassType &Sema) override;

        // Functions
        void visit(SemaClassMethod &Sema) override;
    	void visit(SemaFunction &Sema) override;

        // Variables
    	void visit(SemaClassAttribute &Sema) override;
        void visit(SemaLocalVar &Sema) override;
        void visit(SemaParam &Sema) override;
        void visit(SemaClassInstance &Sema) override;
        void visit(SemaErrorHandler &Sema) override;

        // Expressions
    	void visit(SemaMember &Sema) override;
        void visit(SemaCall &Sema) override;
        void visit(SemaUnary &Sema) override;
        void visit(SemaBinary &Sema) override;
        void visit(SemaTernary &Sema) override;
        void visit(SemaCast &Sema) override;

        // Values
        void visit(SemaBoolValue &Sema) override;
        void visit(SemaIntValue &Sema) override;
        void visit(SemaFloatValue &Sema) override;
        void visit(SemaStringValue &Sema) override;
        void visit(SemaArrayValue &Sema) override;
        void visit(SemaStructValue &Sema) override;
        void visit(SemaNullValue &Sema) override;
        void visit(SemaEnumValue &Sema) override;

    private:

    	void GenBlockStmt(ASTBlockStmt *BlockStmt);

    	void GenStmt(ASTStmt * Stmt);

    	void GenStmtDecl(ASTDeclStmt *DeclStmt);

    	void GenStmtExpr(ASTExprStmt *ExprStmt);

    	void GenStmtDelete(ASTDeleteStmt *DeleteStmt);

    	void GenStmtBreak(ASTBreakStmt *BreakStmt);

    	void GenStmtContinue(ASTContinueStmt *ContinueStmt);

    	void GenStmtReturn(ASTReturnStmt *ReturnStmt);

    	void GenStmtHandle(ASTHandleStmt *HandleStmt);

    	void GenFailStmt(ASTFailStmt *FailStmt);

    	void StoreFail(ASTExpr *Expr, CodeGenError * CGE);

    	void GenIfStmt(ASTIfStmt *If);

    	void GenElsifStmt(CodeGenFunctionBase *CGF,
										llvm::BasicBlock *ElsifBB,
										llvm::SmallVector<ASTRuleStmt *, 8>::iterator &It);

    	void GenSwitchStmt(ASTSwitchStmt *Switch);

    	void GenLoopStmt(ASTLoopStmt *Loop);

    	void GenStmtLoopIn(ASTLoopInStmt *LoopIn);

    	std::string toIdentifier(llvm::StringRef Name, SemaNameSpace *NameSpace);

    };
}

#endif //FLY_CODEGEN_MODULE_H
