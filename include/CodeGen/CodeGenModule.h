//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGenModule.h - LLVM IR emission for modules
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
#include "Basic/SourceLocation.h"
#include "Basic/SourceManager.h"
#include "Basic/TargetInfo.h"
#include "Sema/SemaVisitor.h"
#include <Sema/SemaType.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/IRBuilder.h>
#include <string>

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
    class DIBuilder;
    class DICompileUnit;
    class DIFile;
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
    class SemaModule;
    class SemaFunction;
    class SemaClassType;
    class SemaEnumType;
    class SemaFunctionBase;
    class SemaVar;
    class SemaCall;
    class SemaExpr;
    class SemaNameSpace;
    class SemaClassMethod;
    class SemaClassAttribute;
    class SemaImport;
    class SemaType;
    class SemaIntType;
    class SemaFloatType;
    class SemaComplexType;
    class SemaArrayType;
    class SemaErrorType;
    class SemaLocalVar;
    class SemaParam;
    class SemaAlloc;
    class SemaMember;
    class SemaClassInstance;
    class SemaError;
    class SemaUnary;
    class SemaBinary;
    class SemaTernary;
    class SemaCast;
    class SemaBoolValue;
    class SemaIntValue;
    class SemaFloatValue;
    class SemaComplexValue;
    class SemaStringValue;
    class SemaArrayValue;
    class SemaStructValue;
    class SemaNullValue;
    class SemaEnumEntry;
    class SemaStmt;
    class SemaBlockStmt;
    class SemaDeclStmt;
    class SemaExprStmt;
    class SemaReturnStmt;
    class SemaIfStmt;
    class SemaSwitchStmt;
    class SemaLoopStmt;
    class SemaLoopInStmt;
    class SemaDeleteStmt;
    class SemaBreakStmt;
    class SemaContinueStmt;
    class SemaFailStmt;
    class SemaHandleStmt;

    class CodeGenStdLibLLVM;
    class CodeGenStdLibRuntime;
    class CodeGenStdLibCLang;

    class CodeGenModule : public SemaVisitor {

        friend class CodeGen;
        friend class CodeGenFunction;
        friend class CodeGenFunctionBase;
        friend class CodeGenClassMethod;
        friend class CodeGenClass;
        friend class CodeGenClassMethod;
        friend class CodeGenHandle;
    	friend class CodeGenVar;
    	friend class CodeGenError;
    	friend class CodeGenExpr;
    	friend class CodeGenStdLibLLVM;
    	friend class CodeGenStdLibRuntime;
    	friend class CodeGenStdLibCLang;

        // Reference to CodeGen (contains all LLVM types)
        CodeGen &CG;

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

        // Debug info (non-null only when CGOpts.DebugSymbols is true)
        llvm::DIBuilder     *DBuilder  = nullptr;
        llvm::DICompileUnit *DebugCU   = nullptr;
        llvm::DIFile        *DebugFile = nullptr;

        // Source manager for line/column lookup (nullable; null in tests without SM)
        SourceManager *SM = nullptr;

        // Cache: SemaType* → DIType* (avoids duplicate DWARF type entries)
        llvm::DenseMap<SemaType *, llvm::DIType *> DITypeCache;

        // Scope stack: one DILexicalBlock per SemaBlockStmt visit
        llvm::SmallVector<llvm::DIScope *, 4> DebugScopeStack;

    public:

    	llvm::SmallVector<SemaFunctionBase *, 8> Functions;

        // fly.bridge.CLang: maps each CLang instance alloca → lib string literal.
        // Populated at new CLang(lib) call sites; consumed by CLang::call() codegen.
        llvm::DenseMap<llvm::Value *, std::string> CLangLibMap;

        // Stack for tracking break/continue targets in loops and switches
        llvm::SmallVector<llvm::BasicBlock *, 8> BreakTargetStack;
        llvm::SmallVector<llvm::BasicBlock *, 8> ContinueTargetStack;

        // Stack of SemaBlockStmt pointers; one entry pushed per SemaBlockStmt visit.
        // Each block owns its SemaAlloc list; cleanup iterates the top N frames.
        llvm::SmallVector<SemaBlockStmt *, 8> AllocCleanupStack;
        // Depth of AllocCleanupStack at each loop entry (for break/continue cleanup)
        llvm::SmallVector<size_t, 8> BreakCleanupDepth;
        llvm::SmallVector<size_t, 8> ContinueCleanupDepth;

        SemaFunctionBase *CurrentFunction = nullptr;

        SemaModule *CurrentSemaModule = nullptr;

    	CodeGenError *CurrentErrorHandler = nullptr;

    	llvm::BasicBlock *CurrentHandleBB = nullptr;

    	llvm::BasicBlock *CurrentSafeBB = nullptr;

        CodeGenModule(CodeGen &CG, DiagnosticsEngine &Diags, StringRef Name, llvm::LLVMContext &LLVMCtx,
                      TargetInfo &Target, CodeGenOptions &CGOpts, SourceManager *SM = nullptr);

        virtual ~CodeGenModule();

        DiagnosticBuilder Diag(unsigned DiagID);

        // Emit cleanup for all scope-managed allocations in the top `frames` frames.
        // Handles both smart pointers (free / shared_release) and heap-owned strings (free).
        void EmitAllocCleanup(size_t frames);

        // Emit inline retain/release for shared pointer reference counting
        void EmitSharedRetain(llvm::Value *DataPtr);
        void EmitSharedRelease(llvm::Value *DataPtr);

        llvm::Module *getModule() const;

        void FinalizeDebugInfo();

        void EmitDebugLocation(const SourceLocation &Loc);

        llvm::DIType *GetOrCreateDIType(SemaType *Ty);

    	TargetInfo &getTarget();

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
        void visit(SemaComplexType &Sema) override;
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
        void visit(SemaError &Sema) override;

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
        void visit(SemaComplexValue &Sema) override;
        void visit(SemaStringValue &Sema) override;
        void visit(SemaArrayValue &Sema) override;
        void visit(SemaStructValue &Sema) override;
        void visit(SemaNullValue &Sema) override;
        void visit(SemaUnsetValue &Sema) override;
        void visit(SemaEnumEntry &Sema) override;
        void visit(SemaEnumList &Sema) override;

        // Statements
        void visit(SemaBlockStmt &Sema) override;
        void visit(SemaDeclStmt &Sema) override;
        void visit(SemaExprStmt &Sema) override;
        void visit(SemaReturnStmt &Sema) override;
        void visit(SemaIfStmt &Sema) override;
        void visit(SemaSwitchStmt &Sema) override;
        void visit(SemaLoopStmt &Sema) override;
        void visit(SemaLoopInStmt &Sema) override;
        void visit(SemaDeleteStmt &Sema) override;
        void visit(SemaBreakStmt &Sema) override;
        void visit(SemaContinueStmt &Sema) override;
        void visit(SemaFailStmt &Sema) override;
        void visit(SemaHandleStmt &Sema) override;

    private:

    	void StoreFail(SemaExpr *Expr, CodeGenError * CGE);


    	std::string toIdentifier(llvm::StringRef Name, SemaNameSpace *NameSpace);

    };
}

#endif //FLY_CODEGEN_MODULE_H
