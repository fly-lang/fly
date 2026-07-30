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
// SemaModule must be a complete type here: the OwnedModules member instantiates
// SmallPtrSet<const SemaModule*, 8>, and MSVC eagerly evaluates alignof(SemaModule)
// via PointerLikeTypeTraits (a forward declaration fails to build on Windows).
#include <Sema/SemaModule.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SmallPtrSet.h>
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
    class SemaArrayAccess;
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
    class SemaTestStmt;
    class SemaCaseStmt;

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

        // Classes whose offset-dependent build steps were deferred because their
        // struct layout was not yet available during a cyclic build. Drained at
        // the end of GenerateDeclarations once all types are sized.
        llvm::SmallVector<CodeGenClass *, 8> DeferredClassFinish;

        // fly.bridge.CLang: maps each CLang instance alloca → lib string literal.
        // Populated at new CLang(lib) call sites; consumed by CLang::call() codegen.
        llvm::DenseMap<llvm::Value *, std::string> CLangLibMap;
        // Secondary map by SemaVar* for class-field CLang instances.
        // Used when the GEP pointer differs between assignment and call sites.
        llvm::DenseMap<SemaVar *, std::string> CLangLibMapBySema;

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

        // The set of SemaModules being lowered into THIS llvm::Module (the input
        // files of a single `fly` invocation). A class is "external" — emitted as a
        // declaration only — when its module is NOT in this set (it comes from an
        // imported library). Without this, compiling several files together would
        // wrongly treat a class first referenced from a sibling file as external.
        llvm::SmallPtrSet<const SemaModule *, 8> OwnedModules;

    	CodeGenError *CurrentErrorHandler = nullptr;

    	// Set while emitting a fly.runtime C-ABI function whose LLVM return type is a
    	// value (its out param): a bare `return` must `ret load(out)`, not `ret void`.
    	llvm::Value *CABIReturnPtr = nullptr;
    	llvm::Type  *CABIReturnTy  = nullptr;

    	llvm::BasicBlock *CurrentHandleBB = nullptr;

    	llvm::BasicBlock *CurrentSafeBB = nullptr;

    	// Non-null while generating a suite `case` body: target of the
    	// fail-fast checks emitted after every statement (see visit(SemaBlockStmt)).
    	llvm::BasicBlock *CurrentCaseEndBB = nullptr;

        CodeGenModule(CodeGen &CG, DiagnosticsEngine &Diags, StringRef Name, llvm::LLVMContext &LLVMCtx,
                      TargetInfo &Target, CodeGenOptions &CGOpts, SourceManager *SM = nullptr);

        virtual ~CodeGenModule();

        DiagnosticBuilder Diag(unsigned DiagID);

        // Emit cleanup for all scope-managed allocations in the top `frames` frames.
        // Handles both smart pointers (free / shared_release) and heap-owned strings (free).
        void EmitAllocCleanup(size_t frames);

        // Allocate a reference-counted heap block, [i64 refcount | payload], and
        // return the pointer to the PAYLOAD (8 bytes past the header) with the count
        // initialised to 1. Same layout `new shared` builds inline, so the same
        // retain/release below work on the result. Used for array buffers, whose
        // readers only ever see the payload pointer and so need no change.
        llvm::Value *EmitRCBufferAlloc(llvm::Value *ByteSize);

        // Release one reference to an array buffer, given the address of the %array
        // fat pointer that holds it. Null-guarded: an array slot legitimately holds
        // null (`int[0]`, a runtime size <= 0, a declaration an early `return` never
        // reached), which is why this cannot just call EmitSharedRelease.
        // ArrayType is the slot's static type and is what makes a NESTED array
        // (`int[][]`) release its inner buffers too — one buffer and one count per
        // level. Passing null releases this level only.
        void EmitArrayRelease(llvm::Value *ArraySlotPtr, SemaType *ArrayType = nullptr);

        // Take one more reference to an array buffer, given its DATA pointer (the
        // value stored in field 0 of the fat pointer). Null-guarded for the same
        // reason as the release.
        void EmitArrayRetain(llvm::Value *DataPtr);

        // A stack slot allocated ONCE per call, in the function's entry block. An
        // `alloca` emitted at the current insert point becomes a DYNAMIC allocation
        // when that point is inside a loop: the frame grows every iteration and the
        // stack eventually overflows. Any scratch slot emitted from a statement or
        // expression must come from here.
        llvm::AllocaInst *CreateEntryAlloca(llvm::Type *Ty, const llvm::Twine &Name = "");

        // The LLVM type ONE array element occupies in the buffer. It is the element's
        // own codegen type for everything that has value semantics, but a POINTER for a
        // class or interface: an array of objects holds references, so freeing the
        // buffer drops the pointers and never the objects (docs — an object is freed by
        // whoever is responsible for it). Every place that walks a buffer — the sized
        // allocation, the literal writer, the subscript, the for-in — must size and
        // stride with THIS, or they disagree about the layout.
        // Structs stay inline: they are values everywhere else in the language.
        llvm::Type *GetArrayElementStorageType(SemaType *ElemType);

        // Fill a freshly allocated buffer of class references with one `new C()` per
        // index — malloc + zero + init_ctor + the no-argument constructor, the same
        // sequence `new` emits. The instances belong to the PROGRAM, not to the array:
        // releasing the array never touches them.
        void EmitArrayElementsNew(llvm::Value *DataPtr, llvm::Value *Size,
                                  SemaClassType *ElemClass);

        // Emit inline retain/release for shared pointer reference counting
        void EmitSharedRetain(llvm::Value *DataPtr);
        void EmitSharedRelease(llvm::Value *DataPtr);

        // Emit main()'s exit protocol (load error.code → err_print when non-zero →
        // `ret i32 code`). Shared by the end-of-body epilogue and by `fail`/`return`
        // statements emitted directly inside main, whose LLVM type is the i32 C
        // entry point — a callee-style `ret void` there breaks the function.
        void EmitMainErrorExit(CodeGenError *CGE, llvm::Function *Fn);

        // Raise a runtime error with a fixed code from an EXPRESSION context, using
        // the same protocol as a `fail` statement: store the code, run the alloc
        // cleanup, then leave the function the way the enclosing context requires —
        // main's i32 exit, a plain `ret void`, or a branch to the handle's safe
        // block. Kept here, in one place, precisely because that last branch is the
        // interaction B032 got wrong; a second copy inside CodeGenExpr would be a
        // second chance to get the CFG wrong. Used by the array bounds check.
        void EmitFailWithCode(uint32_t Code);

        llvm::Module *getModule() const;

        void FinalizeDebugInfo();

        void EmitDebugLocation(const SourceLocation &Loc);

        llvm::DIType *GetOrCreateDIType(SemaType *Ty);

    	TargetInfo &getTarget();

    	llvm::LLVMContext &getLLVMCtx() const;

    	llvm::IRBuilder<> *getBuilder() const;

        // SemaVisitor interface implementation
        void visit(SemaModule &Sema) override;

        // Two-phase generation split out of visit(SemaModule) so that several
        // SemaModules (e.g. multiple input files of the same namespace) can be
        // lowered into ONE llvm::Module: declare every module's nodes first, then
        // generate all queued function bodies. Cross-file calls then resolve
        // intra-module instead of producing bodyless stubs.
        void GenerateDeclarations(SemaModule &Sema);
        void GenerateBodies();

        // Register a module as owned by this CGM (local, not an external library).
        void addOwnedModule(const SemaModule *M) { OwnedModules.insert(M); }
        bool ownsModule(const SemaModule *M) const { return OwnedModules.contains(M); }
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
        void visit(SemaArrayAccess &Sema) override;
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
        void visit(SemaEnumAccessor &Sema) override;

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

        // Test block and suite case codegen
        void visit(SemaTestStmt &Sema) override;
        void visit(SemaCaseStmt &Sema) override;

    private:

        // Called from visit(SemaClassType) when ClassKind == SUITE
        void EmitSuite(SemaClassType &Sema);

    	// Returns true when the expression wrote the integer code field, so
    	// visit(SemaFailStmt) can default it to 1 otherwise.
    	bool StoreFail(SemaExpr *Expr, CodeGenError * CGE);

    	// Evaluate a condition expression to an i1. An `error`-typed condition
    	// (`if (err)`) is true when an error is recorded (error.code != 0).
    	llvm::Value *EmitCondition(SemaExpr *Expr);

    	// i1 "an error is recorded" from an %error struct pointer.
    	llvm::Value *EmitErrorIsSet(llvm::Value *ErrPtr);


    	std::string toIdentifier(llvm::StringRef Name, SemaNameSpace *NameSpace);

    };
}

#endif //FLY_CODEGEN_MODULE_H
