//===-------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenModule.cpp - Emit LLVM Code from ASTs for a Module
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
/// \file
/// Defines the fly::CodeGenModule builder.
/// This builds an AST and converts it to LLVM Code.
///
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenModule.h"

#include "Basic/Debug.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenArrayValue.h"
#include "CodeGen/CodeGenClass.h"
#include "CodeGen/CodeGenEnum.h"
#include "CodeGen/CodeGenEnumEntry.h"
#include "CodeGen/CodeGenError.h"
#include "CodeGen/CodeGenExpr.h"
#include "CodeGen/CodeGenFunction.h"
#include "Sema/SemaBinary.h"
#include "Sema/SemaCast.h"
#include "Sema/SemaNameSpace.h"
#include "Sema/SemaParam.h"
#include "Sema/SemaTernary.h"
#include "Sema/SemaUnary.h"
#include "Sema/SemaEnumEntry.h"
#include "Sema/SemaBlockStmt.h"
#include "Sema/SemaDeclStmt.h"
#include "Sema/SemaAlloc.h"
#include "Sema/SemaSmartAlloc.h"
#include "Sema/SemaStringAlloc.h"
#include "AST/ASTCall.h"
#include "AST/ASTStmt.h"
#include "Sema/SemaExprStmt.h"
#include "Sema/SemaReturnStmt.h"
#include "Sema/SemaIfStmt.h"
#include "Sema/SemaSwitchStmt.h"
#include "Sema/SemaLoopStmt.h"
#include "Sema/SemaLoopInStmt.h"
#include "Sema/SemaDeleteStmt.h"
#include "Sema/SemaBreakStmt.h"
#include "Sema/SemaContinueStmt.h"
#include "Sema/SemaFailStmt.h"
#include "Sema/SemaHandleStmt.h"
#include <Sema/SemaCall.h>
#include <Sema/SemaClassAttribute.h>
#include <Sema/SemaClassMethod.h>
#include <Sema/SemaClassType.h>
#include <Sema/SemaEnumType.h>
#include <Sema/SemaEnumEntry.h>
#include <Sema/SemaEnumList.h>
#include <Sema/SemaError.h>
#include <Sema/SemaFunction.h>
#include <Sema/SemaLocalVar.h>
#include <Sema/SemaVar.h>
#include <Sema/SemaMember.h>
#include <Sema/SemaModule.h>
#include <Sema/SemaValue.h>
#include <llvm/IR/Instructions.h>
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Signals.h"
#include "llvm/BinaryFormat/Dwarf.h"

using namespace fly;

CodeGenModule::CodeGenModule(CodeGen &CG, DiagnosticsEngine &Diags, StringRef Name, llvm::LLVMContext &LLVMCtx,
                             TargetInfo &Target, CodeGenOptions &CGOpts, SourceManager *SM) :
        CG(CG),
        Diags(Diags),
        Target(Target),
        LLVMCtx(LLVMCtx),
        Module(new llvm::Module(Name, LLVMCtx)),
        Builder(new llvm::IRBuilder<>(LLVMCtx)),
        CGOpts(CGOpts),
        SM(SM),
        CurrentFunction(nullptr) {

    // Types are now in CodeGen (CG), no need to initialize them here

	// Add Dummy Global Variable which use the Error Type to be sure that the type is in top of the Module
	new llvm::GlobalVariable(*Module, CG.ErrorTy, true, llvm::GlobalValue::ExternalLinkage,
		nullptr, "error");

    // If debug info or coverage generation is enabled, create the CGDebugInfo
    // object.
//    if (CGOpts.getDebugInfo() != codegenoptions::NoDebugInfo ||
//            CGOpts.EmitGcovArcs || CGOpts.EmitGcovNotes)
//        DebugInfo.reset(new CGDebugInfo(*this)); TODO

    // Configure Module
    Module->setTargetTriple(Target.getTriple().getTriple());
    Module->setDataLayout(Target.getDataLayout());
    const auto &SDKVersion = Target.getSDKVersion();
    if (!SDKVersion.empty())
        Module->setSDKVersion(SDKVersion);


    // TODO Add dependencies, Linker Options

    if (CGOpts.DebugSymbols) {
        DBuilder = new llvm::DIBuilder(*Module);
        llvm::StringRef FullPath = Module->getName();
        llvm::StringRef Dir  = llvm::sys::path::parent_path(FullPath);
        llvm::StringRef File = llvm::sys::path::filename(FullPath);
        DebugFile = DBuilder->createFile(File.empty() ? FullPath : File,
                                         Dir.empty()  ? "."      : Dir);
        DebugCU = DBuilder->createCompileUnit(
            llvm::dwarf::DW_LANG_C,
            DebugFile,
            "Fly Compiler",
            /*isOptimized=*/false,
            /*Flags=*/"",
            /*RuntimeVersion=*/0
        );
    }
}

CodeGenModule::~CodeGenModule() {
    // Ensure stacks are clean (they should be empty at this point)
    BreakTargetStack.clear();
    ContinueTargetStack.clear();
    delete Builder;
    delete DBuilder;
    // Note: Module ownership is transferred to caller via getModule(), so we don't delete it here
}

void CodeGenModule::FinalizeDebugInfo() {
    if (DBuilder)
        DBuilder->finalize();
}

void CodeGenModule::EmitDebugLocation(const SourceLocation &Loc) {
    if (!DBuilder || !SM) return;
    llvm::BasicBlock *BB = Builder->GetInsertBlock();
    if (!BB) return;
    llvm::Function *Fn = BB->getParent();
    if (!Fn || !Fn->getSubprogram()) return;
    unsigned Line = SM->getSpellingLineNumber(Loc);
    unsigned Col  = SM->getSpellingColumnNumber(Loc);
    Builder->SetCurrentDebugLocation(
        llvm::DILocation::get(LLVMCtx, Line, Col, Fn->getSubprogram()));
}

llvm::DIType *CodeGenModule::GetOrCreateDIType(SemaType *Ty) {
    if (!DBuilder || !Ty) return nullptr;

    auto It = DITypeCache.find(Ty);
    if (It != DITypeCache.end()) return It->second;

    llvm::DIType *DIT = nullptr;
    unsigned PtrBits = Target.getPointerWidth(0);

    if (Ty->isVoid()) {
        DIT = nullptr;
    } else if (Ty->isBool()) {
        DIT = DBuilder->createBasicType("bool", 1, llvm::dwarf::DW_ATE_boolean);
    } else if (Ty->isInteger()) {
        auto *IT = static_cast<SemaIntType *>(Ty);
        switch (IT->getIntKind()) {
            case SemaIntTypeKind::TYPE_BYTE:
                DIT = DBuilder->createBasicType("byte",   8,  llvm::dwarf::DW_ATE_unsigned); break;
            case SemaIntTypeKind::TYPE_USHORT:
                DIT = DBuilder->createBasicType("ushort", 16, llvm::dwarf::DW_ATE_unsigned); break;
            case SemaIntTypeKind::TYPE_UINT:
                DIT = DBuilder->createBasicType("uint",   32, llvm::dwarf::DW_ATE_unsigned); break;
            case SemaIntTypeKind::TYPE_ULONG:
                DIT = DBuilder->createBasicType("ulong",  64, llvm::dwarf::DW_ATE_unsigned); break;
            case SemaIntTypeKind::TYPE_SHORT:
                DIT = DBuilder->createBasicType("short",  16, llvm::dwarf::DW_ATE_signed);   break;
            case SemaIntTypeKind::TYPE_INT:
                DIT = DBuilder->createBasicType("int",    32, llvm::dwarf::DW_ATE_signed);   break;
            case SemaIntTypeKind::TYPE_LONG:
                DIT = DBuilder->createBasicType("long",   64, llvm::dwarf::DW_ATE_signed);   break;
        }
    } else if (Ty->isFloat()) {
        auto *FT = static_cast<SemaFloatType *>(Ty);
        bool isDouble = (FT->getFloatKind() == SemaFloatTypeKind::TYPE_DOUBLE);
        DIT = DBuilder->createBasicType(
            isDouble ? "double" : "float",
            isDouble ? 64 : 32, llvm::dwarf::DW_ATE_float);
    } else if (Ty->isString()) {
        llvm::DIType *CharTy = DBuilder->createBasicType(
            "char", 8, llvm::dwarf::DW_ATE_unsigned_char);
        DIT = DBuilder->createPointerType(CharTy, PtrBits);
    } else {
        // Classes, arrays, enums: emit an unspecified opaque type for now
        DIT = DBuilder->createUnspecifiedType(Ty->getName());
    }

    DITypeCache[Ty] = DIT;
    return DIT;
}

DiagnosticBuilder CodeGenModule::Diag(unsigned DiagID) {
    if (DebugLog && DiagID == diag::err_invalid_behavior)
        llvm::sys::PrintStackTrace(llvm::errs());
    return Diags.Report(DiagID);
}

llvm::Module *CodeGenModule::getModule() const {
    return Module;
}

TargetInfo &CodeGenModule::getTarget() {
	return Target;
}

llvm::LLVMContext &CodeGenModule::getLLVMCtx() const {
	return LLVMCtx;
}

llvm::IRBuilder<> *CodeGenModule::getBuilder() const {
	return Builder;
}

// =============================================================================
// SemaVisitor interface implementation
// =============================================================================

void CodeGenModule::visit(SemaModule &Sema) {
    CurrentSemaModule = &Sema;

    // Generate all nodes (functions, classes, enums)
    for (auto &Node : Sema.getNodes()) {
        Node->accept(*this);
    }

    // Generate Function Bodies in a second pass
	for (auto &FB : Functions) {
		FB->accept(*this);
	}
}

void CodeGenModule::visit(SemaNameSpace &Sema) {
	// NameSpaces are not directly generated - they are organizational constructs
}

void CodeGenModule::visit(SemaImport &Sema) {
	// Imports are resolved during semantic analysis, not code generation
}

void CodeGenModule::visit(SemaBoolType &Sema) {
    if (Sema.getCodeGen() == nullptr) {
		CodeGenType *CG = new CodeGenType(this);
    	CG->GenType(Sema);
		Sema.setCodeGen(CG);
	}
}

void CodeGenModule::visit(SemaIntType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenType *CG = new CodeGenType(this);
		CG->GenType(Sema);
		Sema.setCodeGen(CG);
	}
}

void CodeGenModule::visit(SemaFloatType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenType *CG = new CodeGenType(this);
		CG->GenType(Sema);
		Sema.setCodeGen(CG);
	}
}

void CodeGenModule::visit(SemaComplexType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenType *CG = new CodeGenType(this);
		CG->GenType(Sema);
		Sema.setCodeGen(CG);
	}
}

void CodeGenModule::visit(SemaArrayType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenType *CG = new CodeGenType(this);
		CG->GenType(Sema);
		Sema.setCodeGen(CG);
	}
}

void CodeGenModule::visit(SemaErrorType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenType *CG = new CodeGenType(this);
		CG->GenType(Sema);
		Sema.setCodeGen(CG);
	}
}

void CodeGenModule::visit(SemaVoidType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenType *CG = new CodeGenType(this);
		CG->GenType(Sema);
		Sema.setCodeGen(CG);
	}
}

void CodeGenModule::visit(SemaStringType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenType *CG = new CodeGenType(this);
		CG->GenType(Sema);
		Sema.setCodeGen(CG);
	}
}

void CodeGenModule::visit(SemaEnumType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenEnum *CGE = new CodeGenEnum(this, &Sema, false);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaClassType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		bool isExternal = (CurrentSemaModule != nullptr &&
		                   &Sema.getModule() != CurrentSemaModule);
		CodeGenClass *CGC = new CodeGenClass(this, &Sema, isExternal);
		Sema.setCodeGen(CGC);
	}
}

void CodeGenModule::visit(SemaClassMethod &Sema) {
	CurrentFunction = &Sema;
	if (Sema.getCodeGen()) {
		Sema.getCodeGen()->GenBody();
	}
}

void CodeGenModule::visit(SemaFunction &Sema) {
	CurrentFunction = &Sema;
	if (Sema.getCodeGen() == nullptr) {
		CodeGenFunction *CGF = new CodeGenFunction(this, &Sema, false);
		Sema.setCodeGen(CGF);
		Functions.push_back(&Sema);
	} else {
		Sema.getCodeGen()->GenBody();
	}
}

void CodeGenModule::visit(SemaClassAttribute &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		Sema.getType()->accept(*this);
		llvm::Type *T = Sema.getType()->getCodeGen()->getType();
		CodeGenVar *CGV = new CodeGenVar(this, &Sema, T);
		Sema.setCodeGen(CGV);
	}

	// When accessed inside a non-static class method as a bare name (e.g. `fd`, not `this.fd`),
	// compute a GEP to the correct struct field so Load/Store hit the actual field, not the alloca.
	if (!Sema.isStatic() && CurrentFunction &&
	    CurrentFunction->getKind() == SemaKind::METHOD) {
		SemaClassMethod *Method = static_cast<SemaClassMethod *>(CurrentFunction);
		if (!Method->isStatic()) {
			llvm::Value *Instance = Method->getThis()->getCodeGen()->getValue();
			llvm::StructType *StructTy = Sema.getClass().getCodeGen()->getType();
			size_t FieldIdx = Sema.getCodeGen()->getIndex();
			llvm::Value *FieldPtr = Builder->CreateInBoundsGEP(StructTy, Instance,
				{CodeGen::Zero, llvm::ConstantInt::get(CodeGen::Int32Ty, FieldIdx)});
			Sema.getCodeGen()->setPointer(FieldPtr);
		}
	}
}

void CodeGenModule::visit(SemaLocalVar &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		Sema.getType()->accept(*this);
		llvm::Type *T = Sema.getType()->getCodeGen()->getType();
		CodeGenVar *CGV = new CodeGenVar(this, &Sema, T);
		Sema.setCodeGen(CGV);
	}
}

void CodeGenModule::visit(SemaParam &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		Sema.getType()->accept(*this);
		llvm::Type *T = Sema.getType()->getCodeGen()->getType();
		CodeGenVar *CGV = new CodeGenVar(this, &Sema, T);
		Sema.setCodeGen(CGV);
	}
}

void CodeGenModule::visit(SemaClassInstance &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		Sema.getType()->accept(*this);
		llvm::Type *T = Sema.getType()->getCodeGen()->getType();
		CodeGenVar *CGV = new CodeGenVar(this, &Sema, T);
		Sema.setCodeGen(CGV);
	}
}

void CodeGenModule::visit(SemaError &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		Sema.getType()->accept(*this);
		llvm::Value *ErrorHandler = Builder->CreateAlloca(CG.ErrorPtrTy);
		CodeGenError *CGE = new CodeGenError(this, &Sema, ErrorHandler);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaMember &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		// Only set wrapper if GenExpr didn't already set the CodeGen (e.g. CodeGenVar for attributes)
		if (Sema.getCodeGen() == nullptr) {
			Sema.setCodeGen(CGE);
		}
	}
}

void CodeGenModule::visit(SemaCall &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaUnary &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaBinary &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaTernary &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaCast &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaBoolValue &Sema) {
    if (Sema.getCodeGen() == nullptr) {
    	CodeGenExpr *CGE = new CodeGenExpr(this);
    	CGE->GenExpr(&Sema);
    	Sema.setCodeGen(CGE);
    }
}

void CodeGenModule::visit(SemaIntValue &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaFloatValue &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaComplexValue &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaStringValue &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaArrayValue &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenArrayValue *CGE = new CodeGenArrayValue(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaStructValue &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaNullValue &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaUnsetValue &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaEnumEntry &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenEnumEntry *CGE = new CodeGenEnumEntry(this, &Sema);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaEnumList &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenArrayValue *CGE = new CodeGenArrayValue(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

// ─── Sema Statement visitors (delegate to AST-based Gen methods) ─────────────

void CodeGenModule::EmitSharedRetain(llvm::Value *DataPtr) {
	llvm::Type *I8Ty  = llvm::Type::getInt8Ty(LLVMCtx);
	llvm::Type *I64Ty = llvm::Type::getInt64Ty(LLVMCtx);
	// header is 8 bytes before the data pointer
	llvm::Value *Header = Builder->CreateGEP(I8Ty, DataPtr,
		llvm::ConstantInt::getSigned(llvm::Type::getInt64Ty(LLVMCtx), -8), "shrd_hdr");
	llvm::Value *RC  = Builder->CreateLoad(I64Ty, Header, "shrd_rc");
	llvm::Value *RC1 = Builder->CreateAdd(RC, llvm::ConstantInt::get(I64Ty, 1), "shrd_rc1");
	Builder->CreateStore(RC1, Header);
}

void CodeGenModule::EmitSharedRelease(llvm::Value *DataPtr) {
	llvm::Type *I8Ty  = llvm::Type::getInt8Ty(LLVMCtx);
	llvm::Type *I64Ty = llvm::Type::getInt64Ty(LLVMCtx);
	llvm::Function *Fn = CurrentFunction->getCodeGen()->getFunction();

	llvm::Value *Header = Builder->CreateGEP(I8Ty, DataPtr,
		llvm::ConstantInt::getSigned(I64Ty, -8), "shrd_hdr");
	llvm::Value *RC  = Builder->CreateLoad(I64Ty, Header, "shrd_rc");
	llvm::Value *RC1 = Builder->CreateSub(RC, llvm::ConstantInt::get(I64Ty, 1), "shrd_rc1");
	Builder->CreateStore(RC1, Header);

	llvm::BasicBlock *FreeBB = llvm::BasicBlock::Create(LLVMCtx, "shrd_free", Fn);
	llvm::BasicBlock *DoneBB = llvm::BasicBlock::Create(LLVMCtx, "shrd_done", Fn);
	llvm::Value *IsZero = Builder->CreateICmpEQ(RC1, llvm::ConstantInt::get(I64Ty, 0));
	Builder->CreateCondBr(IsZero, FreeBB, DoneBB);

	Builder->SetInsertPoint(FreeBB);
	llvm::FunctionCallee FreeFn = Module->getOrInsertFunction(
		"free",
		llvm::FunctionType::get(llvm::Type::getVoidTy(LLVMCtx),
			{llvm::PointerType::getUnqual(LLVMCtx)}, false));
	Builder->CreateCall(FreeFn, {Header});
	Builder->CreateBr(DoneBB);

	Builder->SetInsertPoint(DoneBB);
}

void CodeGenModule::EmitAllocCleanup(size_t frames) {
	if (frames == 0 || AllocCleanupStack.empty()) return;
	llvm::FunctionCallee FreeFn;
	// Emit in reverse order (innermost scope first)
	size_t depth = AllocCleanupStack.size();
	size_t limit = (frames < depth) ? depth - frames : 0;
	for (size_t i = depth; i-- > limit;) {
		for (SemaAlloc *Alloc : AllocCleanupStack[i]->getAllocs()) {
			if (Alloc->getKind() == SemaAllocKind::SMART) {
				SemaSmartAlloc *SA = static_cast<SemaSmartAlloc *>(Alloc);
				if (SA->isUnique() || SA->isWeak()) {
					if (!FreeFn) {
						FreeFn = Module->getOrInsertFunction(
							"free",
							llvm::FunctionType::get(
								llvm::Type::getVoidTy(LLVMCtx),
								{llvm::PointerType::getUnqual(LLVMCtx)},
								false));
					}
					// The SemaCall CodeGen value is the heap pointer returned by init_ctor.
					llvm::Value *Ptr = SA->getCall()->getCodeGen()->getValue();
					Builder->CreateCall(FreeFn, {Ptr});
				} else if (SA->isShared()) {
					// Call->getValue() is always the canonical data pointer for this entry,
					// whether it is an original allocation or a copy reference.
					EmitSharedRelease(SA->getCall()->getCodeGen()->getValue());
				}
			} else if (Alloc->getKind() == SemaAllocKind::STRING) {
				SemaStringAlloc *SSA = static_cast<SemaStringAlloc *>(Alloc);
				SemaVar *Var = SSA->getVar();
				if (!Var->getCodeGen()) continue;
				if (!FreeFn) {
					FreeFn = Module->getOrInsertFunction(
						"free",
						llvm::FunctionType::get(
							llvm::Type::getVoidTy(LLVMCtx),
							{llvm::PointerType::getUnqual(LLVMCtx)},
							false));
				}
				llvm::Value *StrVal = Var->getCodeGen()->Load();
				llvm::Value *StrPtr = Builder->CreateExtractValue(StrVal, 0, "hs_ptr");
				Builder->CreateCall(FreeFn, {StrPtr});
			}
		}
	}
}

void CodeGenModule::visit(SemaBlockStmt &Sema) {
	AllocCleanupStack.push_back(&Sema);
	for (SemaStmt *Stmt : Sema.getContent()) {
		Stmt->accept(*this);
	}
	EmitAllocCleanup(1);
	AllocCleanupStack.pop_back();
}

void CodeGenModule::visit(SemaDeclStmt &Sema) {
	FLY_DEBUG_SCOPE("CodeGenModule", "visit(SemaDeclStmt)");
	EmitDebugLocation(Sema.getAST()->getLocation());

	// Get the CodeGenVar for the Local Variable
	CodeGenVar *CGV = Sema.getVar()->getCodeGen();

	// Declaration may be with initialization
	if (Sema.getExpr()) {
		Sema.getExpr()->accept(*this);
		// Emit retain for shared copies: the expression is a variable reference
		// (not a CALL_NEW_SHARED), so the SA wraps another variable's allocation call.
		SemaSmartAlloc *SA = Sema.getVar()->getSmartAlloc();
		if (SA && SA->isShared()) {
			SemaExpr *E = Sema.getExpr();
			SemaExpr *Rhs = (E->getKind() == SemaKind::BINARY)
			                ? static_cast<SemaBinary *>(E)->getRight() : E;
			bool isFreshAlloc = Rhs->getKind() == SemaKind::CALL &&
			                    static_cast<SemaCall *>(Rhs)->getAST().getCallKind()
			                        == ASTCallKind::CALL_NEW_SHARED;
			if (!isFreshAlloc)
				EmitSharedRetain(CGV->Load());
		}
	} else {
		CGV->StoreDefaultValue();
	}
}

void CodeGenModule::visit(SemaExprStmt &Sema) {
	FLY_DEBUG_SCOPE("CodeGenModule", "visit(SemaExprStmt)");
	EmitDebugLocation(Sema.getAST()->getLocation());

	Sema.getExpr()->accept(*this);
}

void CodeGenModule::visit(SemaReturnStmt &Sema) {
	FLY_DEBUG_SCOPE("CodeGenModule", "visit(SemaReturnStmt)");
	EmitDebugLocation(Sema.getAST()->getLocation());
	EmitAllocCleanup(AllocCleanupStack.size());
	Builder->CreateRetVoid();
}

void CodeGenModule::visit(SemaIfStmt &Sema) {
	FLY_DEBUG_SCOPE("CodeGenModule", "visit(SemaIfStmt)");
	EmitDebugLocation(Sema.getAST()->getLocation());
	llvm::Function *Fn = CurrentFunction->getCodeGen()->getFunction();

	// If Block - use Sema condition expression
	Sema.getCond()->accept(*this);
	llvm::Value *IfCond = Sema.getCond()->getCodeGen()->getValue();
	llvm::BasicBlock *IfBB = llvm::BasicBlock::Create(LLVMCtx, "ifthen", Fn);

	// Create End block
	llvm::BasicBlock *EndBB = llvm::BasicBlock::Create(LLVMCtx, "endif", Fn);

	if (!Sema.getElse()) {

		if (Sema.getElsif().empty()) { // If ...
			Builder->CreateCondBr(IfCond, IfBB, EndBB);
			Builder->SetInsertPoint(IfBB);
			Sema.getThen()->accept(*this);
			if (!Builder->GetInsertBlock()->getTerminator())
				Builder->CreateBr(EndBB);
		} else { // If - elsif ...
			llvm::BasicBlock *ElsifBB = llvm::BasicBlock::Create(LLVMCtx, "elsif", Fn, EndBB);
			Builder->CreateCondBr(IfCond, IfBB, ElsifBB);

			// Start if-then
			Builder->SetInsertPoint(IfBB);
			Sema.getThen()->accept(*this);
			if (!Builder->GetInsertBlock()->getTerminator())
				Builder->CreateBr(EndBB);

			// Create Elsif Blocks
			unsigned long Size = Sema.getElsif().size();
			for (unsigned long i = 0; i < Size; i++) {
				llvm::BasicBlock *ElsifThenBB = llvm::BasicBlock::Create(LLVMCtx, "elsifthen", Fn, EndBB);

				llvm::BasicBlock *NextElsifBB;
				if (i == Size-1) { // is Last
					NextElsifBB = EndBB;
				} else {
					NextElsifBB = llvm::BasicBlock::Create(LLVMCtx, "elsif", Fn, EndBB);
				}
				SemaRuleStmt ElsifSema = Sema.getElsif()[i];
				Builder->SetInsertPoint(ElsifBB);
				ElsifSema.Expr->accept(*this);
				llvm::Value *ElsifCond = ElsifSema.Expr->getCodeGen()->getValue();
				Builder->CreateCondBr(ElsifCond, ElsifThenBB, NextElsifBB);

				Builder->SetInsertPoint(ElsifThenBB);
				ElsifSema.Stmt->accept(*this);
				if (!Builder->GetInsertBlock()->getTerminator())
					Builder->CreateBr(EndBB);

				ElsifBB = NextElsifBB;
			}
		}

	} else {

		// Create Else block
		llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(LLVMCtx, "else", Fn, EndBB);

		if (Sema.getElsif().empty()) { // If - Else
			Builder->CreateCondBr(IfCond, IfBB, ElseBB);
			Builder->SetInsertPoint(IfBB);
			Sema.getThen()->accept(*this);
			if (!Builder->GetInsertBlock()->getTerminator())
				Builder->CreateBr(EndBB);
		} else { // If - Elsif - Else
			llvm::BasicBlock *ElsifBB = llvm::BasicBlock::Create(LLVMCtx, "elsif", Fn, ElseBB);
			Builder->CreateCondBr(IfCond, IfBB, ElsifBB);

			// Start if-then
			Builder->SetInsertPoint(IfBB);
			Sema.getThen()->accept(*this);
			if (!Builder->GetInsertBlock()->getTerminator())
				Builder->CreateBr(EndBB);

			// Create Elsif Blocks
			unsigned long Size = Sema.getElsif().size();
			for (unsigned long i = 0; i < Size; i++) {
				llvm::BasicBlock *ElsifThenBB = llvm::BasicBlock::Create(LLVMCtx, "elsifthen", Fn, ElseBB);

				llvm::BasicBlock *NextElsifBB;
				if (i == Size-1) { // is Last
					NextElsifBB = ElseBB;
				} else {
					NextElsifBB = llvm::BasicBlock::Create(LLVMCtx, "elsif", Fn, ElseBB);
				}
				SemaRuleStmt ElsifSema = Sema.getElsif()[i];
				Builder->SetInsertPoint(ElsifBB);
				ElsifSema.Expr->accept(*this);
				llvm::Value *ElsifCond = ElsifSema.Expr->getCodeGen()->getValue();
				Builder->CreateCondBr(ElsifCond, ElsifThenBB, NextElsifBB);

				Builder->SetInsertPoint(ElsifThenBB);
				ElsifSema.Stmt->accept(*this);
				if (!Builder->GetInsertBlock()->getTerminator())
					Builder->CreateBr(EndBB);

				ElsifBB = NextElsifBB;
			}
		}

		Builder->SetInsertPoint(ElseBB);
		Sema.getElse()->accept(*this);
		if (!Builder->GetInsertBlock()->getTerminator())
			Builder->CreateBr(EndBB);
	}

	// Continue insertions into End Branch
	Builder->SetInsertPoint(EndBB);
}

void CodeGenModule::visit(SemaSwitchStmt &Sema) {
	FLY_DEBUG_SCOPE("CodeGenModule", "visit(SemaSwitchStmt)");
	EmitDebugLocation(Sema.getAST()->getLocation());
	llvm::Function *Fn = CurrentFunction->getCodeGen()->getFunction();

	// Create End Block
	llvm::BasicBlock *EndBB = llvm::BasicBlock::Create(LLVMCtx, "endswitch", Fn);

	// Push break target onto stack (switches don't have continue)
	BreakTargetStack.push_back(EndBB);

	// Create Expression evaluator for Switch using Sema
	Sema.getExpr()->accept(*this);
	llvm::Value *SwitchVal = Sema.getExpr()->getCodeGen()->getValue();
	llvm::SwitchInst *Inst = Builder->CreateSwitch(SwitchVal, EndBB);

	// Create Cases
	unsigned long Size = Sema.getCases().size();

	llvm::BasicBlock *NextCaseBB = nullptr;
	for (unsigned long i = 0; i < Size; i++) {
		SemaRuleStmt CaseSema = Sema.getCases()[i];
		CaseSema.Expr->accept(*this);
		llvm::Value *CaseVal = CaseSema.Expr->getCodeGen()->getValue();
		llvm::ConstantInt *CaseConst = llvm::cast<llvm::ConstantInt, llvm::Value>(CaseVal);
		llvm::BasicBlock *CaseBB = NextCaseBB == nullptr ?
								   llvm::BasicBlock::Create(LLVMCtx, "case", Fn, EndBB) : NextCaseBB;
		Inst->addCase(CaseConst, CaseBB);
		Builder->SetInsertPoint(CaseBB);
		CaseSema.Stmt->accept(*this);

		// If there is a Next
		if (i + 1 < Size) {
			NextCaseBB = llvm::BasicBlock::Create(LLVMCtx, "case", Fn, EndBB);
			Builder->CreateBr(NextCaseBB);
		} else {
			Builder->CreateBr(EndBB);
		}
	}

	// Create Default
	if (Sema.getDefault()) {
		llvm::BasicBlock *DefaultBB = llvm::BasicBlock::Create(LLVMCtx, "default", Fn, EndBB);
		Inst->setDefaultDest(DefaultBB);
		Builder->SetInsertPoint(DefaultBB);
		Sema.getDefault()->accept(*this);
		Builder->CreateBr(EndBB);
	}

	// Pop break target from stack
	BreakTargetStack.pop_back();

	// Continue insertions into End Branch
	Builder->SetInsertPoint(EndBB);
}

void CodeGenModule::visit(SemaLoopStmt &Sema) {
	FLY_DEBUG_SCOPE("CodeGenModule", "visit(SemaLoopStmt)");
	EmitDebugLocation(Sema.getAST()->getLocation());
	llvm::Function *Fn = CurrentFunction->getCodeGen()->getFunction();

	// Generate Init Statements via Sema
	for (SemaStmt *S : Sema.getInit()) {
		S->accept(*this);
	}

	// Create Condition Block
	llvm::BasicBlock *CondBB = nullptr;
	if (Sema.getCond()) {
		CondBB = llvm::BasicBlock::Create(LLVMCtx, "loopcond", Fn);
	}

	// Create Loop Block
	llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(LLVMCtx, "loop", Fn);

	// Create Post Block
	llvm::BasicBlock *PostBB = nullptr;
	if (!Sema.getPost().empty()) {
		PostBB = llvm::BasicBlock::Create(LLVMCtx, "looppost", Fn);
	}

	// Create End Block
	llvm::BasicBlock *EndBB = llvm::BasicBlock::Create(LLVMCtx, "loopend", Fn);

	// Push break and continue targets onto stacks
	BreakTargetStack.push_back(EndBB);
	BreakCleanupDepth.push_back(AllocCleanupStack.size());
	// Continue target depends on whether we have a Post block or Condition block
	if (PostBB) {
		ContinueTargetStack.push_back(PostBB);
	} else if (CondBB) {
		ContinueTargetStack.push_back(CondBB);
	} else {
		ContinueTargetStack.push_back(LoopBB);
	}
	ContinueCleanupDepth.push_back(AllocCleanupStack.size());

	// Generate Code
	if (CondBB) {
		Builder->CreateBr(CondBB);

		// Create Condition using Sema
		Builder->SetInsertPoint(CondBB);
		Sema.getCond()->accept(*this);
		llvm::Value *Cond = Sema.getCond()->getCodeGen()->getValue();
		Builder->CreateCondBr(Cond, LoopBB, EndBB);
	} else {
		Builder->CreateBr(LoopBB);
	}

	// Add to Loop via Sema body
	Builder->SetInsertPoint(LoopBB);
	if (Sema.getBody()) {
		Sema.getBody()->accept(*this);
	}
	if (PostBB) {
		if (!Builder->GetInsertBlock()->getTerminator())
			Builder->CreateBr(PostBB);

		// Add to Post via Sema
		Builder->SetInsertPoint(PostBB);
		for (SemaStmt *S : Sema.getPost()) {
			S->accept(*this);
		}
		if (!Builder->GetInsertBlock()->getTerminator()) {
			if (CondBB) {
				Builder->CreateBr(CondBB);
			} else {
				Builder->CreateBr(LoopBB);
			}
		}
	} else if (!Builder->GetInsertBlock()->getTerminator()) {
		if (CondBB) {
			Builder->CreateBr(CondBB);
		} else {
			Builder->CreateBr(LoopBB);
		}
	}

	// Pop break and continue targets from stacks
	BreakTargetStack.pop_back();
	BreakCleanupDepth.pop_back();
	ContinueTargetStack.pop_back();
	ContinueCleanupDepth.pop_back();

	// Continue insertions into End Branch
	Builder->SetInsertPoint(EndBB);
}

void CodeGenModule::visit(SemaLoopInStmt &Sema) {
	FLY_DEBUG_SCOPE("CodeGenModule", "visit(SemaLoopInStmt)");
	EmitDebugLocation(Sema.getAST()->getLocation());

	SemaExpr *ListExpr = Sema.getList();
	SemaExpr *ItemExpr = Sema.getItem();
	SemaType *ListType = ListExpr ? ListExpr->getType() : nullptr;

	// Only array iteration is supported currently
	if (!ListType || !ListType->isArray()) {
		if (Sema.getBody())
			Sema.getBody()->accept(*this);
		return;
	}

	SemaArrayType *ArrSemaType = static_cast<SemaArrayType *>(ListType);
	SemaType *ElemSemaType = ArrSemaType->getElementType();
	ElemSemaType->accept(*this);
	llvm::Type *ElemLLVMType = ElemSemaType->getCodeGen()->getType();

	// Obtain the array struct pointer without loading the whole struct.
	// The list var's alloca IS a pointer to the ArrayTy struct.
	llvm::Value *ArrayStructPtr = nullptr;
	SemaKind ListKind = ListExpr->getKind();
	if (ListKind == SemaKind::LOCAL_VAR || ListKind == SemaKind::PARAM_VAR ||
		ListKind == SemaKind::ATTRIBUTE || ListKind == SemaKind::INSTANCE_VAR) {
		SemaVar *ListVar = static_cast<SemaVar *>(ListExpr);
		if (ListVar->getCodeGen())
			ArrayStructPtr = ListVar->getCodeGen()->getPointer();
	}
	if (!ArrayStructPtr) {
		// Fallback: evaluate expression and use its value as the struct pointer
		ListExpr->accept(*this);
		ArrayStructPtr = ListExpr->getCodeGen()->getValue();
	}

	if (!ArrayStructPtr) {
		if (Sema.getBody())
			Sema.getBody()->accept(*this);
		return;
	}

	// Load data pointer: field 0 (i8*)
	llvm::Value *DataPtrField = Builder->CreateStructGEP(CodeGen::ArrayTy, ArrayStructPtr, 0);
	llvm::Value *DataPtr = Builder->CreateLoad(CodeGen::Int8PtrTy, DataPtrField);

	// Cast i8* to ElemType* for GEP arithmetic
	llvm::Value *TypedDataPtr = Builder->CreateBitCast(DataPtr, ElemLLVMType->getPointerTo());

	// Load size: field 1 (IntTy)
	llvm::Value *SizeField = Builder->CreateStructGEP(CodeGen::ArrayTy, ArrayStructPtr, 1);
	llvm::Value *Size = Builder->CreateLoad(CodeGen::IntTy, SizeField);
	// Ensure Size is IntTy width for the loop comparisons
	if (Size->getType() != CodeGen::IntTy)
		Size = Builder->CreateIntCast(Size, CodeGen::IntTy, false);

	// Allocate and zero the loop index
	llvm::AllocaInst *IndexAlloca = Builder->CreateAlloca(CodeGen::IntTy, nullptr, "forin.idx");
	Builder->CreateStore(llvm::ConstantInt::get(CodeGen::IntTy, 0), IndexAlloca);

	llvm::Function *Fn = CurrentFunction->getCodeGen()->getFunction();
	llvm::BasicBlock *CondBB = llvm::BasicBlock::Create(LLVMCtx, "forin.cond", Fn);
	llvm::BasicBlock *BodyBB = llvm::BasicBlock::Create(LLVMCtx, "forin.body", Fn);
	llvm::BasicBlock *EndBB  = llvm::BasicBlock::Create(LLVMCtx, "forin.end",  Fn);

	BreakTargetStack.push_back(EndBB);
	BreakCleanupDepth.push_back(AllocCleanupStack.size());
	ContinueTargetStack.push_back(CondBB);
	ContinueCleanupDepth.push_back(AllocCleanupStack.size());

	Builder->CreateBr(CondBB);

	// Condition: i < size
	Builder->SetInsertPoint(CondBB);
	llvm::Value *Idx = Builder->CreateLoad(CodeGen::IntTy, IndexAlloca, "forin.i");
	llvm::Value *Cond = Builder->CreateICmpSLT(Idx, Size, "forin.cmp");
	Builder->CreateCondBr(Cond, BodyBB, EndBB);

	// Body: assign data[i] to item
	Builder->SetInsertPoint(BodyBB);
	llvm::Value *Idx2 = Builder->CreateLoad(CodeGen::IntTy, IndexAlloca);
	llvm::Value *ElemPtr = Builder->CreateGEP(ElemLLVMType, TypedDataPtr, Idx2, "forin.elem");
	llvm::Value *ElemVal = Builder->CreateLoad(ElemLLVMType, ElemPtr);

	// Store element into loop item variable
	SemaKind ItemKind = ItemExpr ? ItemExpr->getKind() : SemaKind::VALUE;
	if (ItemExpr && (ItemKind == SemaKind::LOCAL_VAR || ItemKind == SemaKind::PARAM_VAR ||
					 ItemKind == SemaKind::ATTRIBUTE || ItemKind == SemaKind::INSTANCE_VAR)) {
		SemaVar *ItemVar = static_cast<SemaVar *>(ItemExpr);
		if (ItemVar->getCodeGen())
			ItemVar->getCodeGen()->Store(ElemVal);
	}

	// Emit loop body
	if (Sema.getBody())
		Sema.getBody()->accept(*this);

	// Increment index and back-edge
	llvm::Value *CurIdx = Builder->CreateLoad(CodeGen::IntTy, IndexAlloca);
	llvm::Value *NextIdx = Builder->CreateAdd(CurIdx, llvm::ConstantInt::get(CodeGen::IntTy, 1));
	Builder->CreateStore(NextIdx, IndexAlloca);
	Builder->CreateBr(CondBB);

	BreakTargetStack.pop_back();
	BreakCleanupDepth.pop_back();
	ContinueTargetStack.pop_back();
	ContinueCleanupDepth.pop_back();

	Builder->SetInsertPoint(EndBB);
}

void CodeGenModule::visit(SemaDeleteStmt &Sema) {
	FLY_DEBUG_SCOPE("CodeGenModule", "visit(SemaDeleteStmt)");
	EmitDebugLocation(Sema.getAST()->getLocation());

	Sema.getExpr()->accept(*this);
	llvm::Value *V = Sema.getExpr()->getCodeGen()->getValue();

	llvm::FunctionCallee FreeFn = Module->getOrInsertFunction(
		"free",
		llvm::FunctionType::get(
			llvm::Type::getVoidTy(LLVMCtx),
			{llvm::PointerType::getUnqual(LLVMCtx)},
			false));

	if (Sema.getExpr()->getType()->isClass()) {
		Builder->CreateCall(FreeFn, {V});
	} else if (Sema.getExpr()->getType()->isString()) {
		llvm::Value *StrPtr = Builder->CreateExtractValue(V, 0);
		Builder->CreateCall(FreeFn, {StrPtr});
	}
}

void CodeGenModule::visit(SemaBreakStmt &Sema) {
	FLY_DEBUG_SCOPE("CodeGenModule", "visit(SemaBreakStmt)");
	EmitDebugLocation(Sema.getAST()->getLocation());
	if (!BreakTargetStack.empty()) {
		size_t loopDepth = BreakCleanupDepth.back();
		EmitAllocCleanup(AllocCleanupStack.size() - loopDepth);
		Builder->CreateBr(BreakTargetStack.back());
	} else {
		Diag(diag::err_invalid_behavior);
	}
}

void CodeGenModule::visit(SemaContinueStmt &Sema) {
	FLY_DEBUG_SCOPE("CodeGenModule", "visit(SemaContinueStmt)");
	EmitDebugLocation(Sema.getAST()->getLocation());
	if (!ContinueTargetStack.empty()) {
		size_t loopDepth = ContinueCleanupDepth.back();
		EmitAllocCleanup(AllocCleanupStack.size() - loopDepth);
		Builder->CreateBr(ContinueTargetStack.back());
	} else {
		Diag(diag::err_invalid_behavior);
	}
}

void CodeGenModule::visit(SemaFailStmt &Sema) {
	FLY_DEBUG_SCOPE("CodeGenModule", "visit(SemaFailStmt)");
	EmitDebugLocation(Sema.getAST()->getLocation());

	if (Sema.getFirst() == nullptr) {
		CurrentErrorHandler->StoreInt(llvm::ConstantInt::get(CG.Int32Ty, 1));
	} else {
		StoreFail(Sema.getFirst(), CurrentErrorHandler);

		if (Sema.getSecond()) {
			StoreFail(Sema.getSecond(), CurrentErrorHandler);

			if (Sema.getThird()) {
				StoreFail(Sema.getThird(), CurrentErrorHandler);
			}
		}
	}

	EmitAllocCleanup(AllocCleanupStack.size());
	if (CurrentHandleBB == nullptr) {
		Builder->CreateRetVoid();
	} else {
		Builder->CreateBr(CurrentSafeBB);
	}
}

void CodeGenModule::visit(SemaHandleStmt &Sema) {
	FLY_DEBUG_SCOPE("CodeGenModule", "visit(SemaHandleStmt)");
	EmitDebugLocation(Sema.getAST()->getLocation());

	// Save parent error handler
	CodeGenError *ParentErrorHandler = CurrentErrorHandler;

	// Create a new error handler using the Sema data
	Sema.getErrorHandler()->accept(*this);
	CurrentErrorHandler = Sema.getErrorHandler()->getCodeGen();

	// Save parent handle statement for nested handles
	llvm::BasicBlock *ParentHandleBB = CurrentHandleBB;
	llvm::BasicBlock *ParentSafeBB = CurrentSafeBB;

	// Take the current Function to create the Handle and Safe blocks
	llvm::Function *Fn = CurrentFunction->getCodeGen()->getFunction();

	// Create Handle and Safe Block
	CurrentHandleBB = llvm::BasicBlock::Create(LLVMCtx, "handle", Fn);
	CurrentSafeBB = llvm::BasicBlock::Create(LLVMCtx, "safe", Fn);
	Builder->CreateBr(CurrentHandleBB);
	Builder->SetInsertPoint(CurrentHandleBB);

	// Generate Handle Block from Sema
	if (Sema.getHandle()) {
		Sema.getHandle()->accept(*this);
	}

	// Generate in Safe Block
	Builder->SetInsertPoint(CurrentSafeBB);

	// Restore parent handles
	CurrentHandleBB = ParentHandleBB;
	CurrentSafeBB = ParentSafeBB;
	CurrentErrorHandler = ParentErrorHandler;
}



void CodeGenModule::StoreFail(SemaExpr *Expr, CodeGenError *CGE) {
	Expr->accept(*this);
	llvm::Value *Value = Expr->getCodeGen()->getValue();
	if (Expr->getType()->isBool()) {
		// Bool is i1, need to zero extend to i32
		llvm::Value *Int32Value = Builder->CreateZExt(Value, CG.Int32Ty);
		CGE->StoreInt(Int32Value);
	} else if (Expr->getType()->isInteger()) {
		SemaIntType *IntType = static_cast<SemaIntType *>(Expr->getType());
		llvm::Value *Int32Value = Value;

		// Get the bit width from the integer kind
		unsigned BitWidth = static_cast<unsigned>(IntType->getIntKind());
		if (IntType->isSigned()) {
			BitWidth += 1; // Signed types have one less bit for the sign
		}

		if (BitWidth < 32) {
			// Smaller than i32, need to extend
			if (IntType->isSigned()) {
				Int32Value = Builder->CreateSExt(Value, CG.Int32Ty);
			} else {
				Int32Value = Builder->CreateZExt(Value, CG.Int32Ty);
			}
		} else if (BitWidth > 32) {
			// Larger than i32 (i64), need to truncate
			Int32Value = Builder->CreateTrunc(Value, CG.Int32Ty);
		}
		// If BitWidth == 32, no conversion needed

		CGE->StoreInt(Int32Value);
	} else if (Expr->getType()->isString()) {
		CGE->StoreString(Value);
	} else if (Expr->getType()->isClass()) {
		CGE->StoreObject(Value);
	} else if (Expr->getType()->isEnum()) {
		CGE->StoreInt(Value);
	}
}


std::string CodeGenModule::toIdentifier(llvm::StringRef Name, SemaNameSpace *NameSpace) {
	FLY_DEBUG_SCOPE("CodeGenModule", "toIdentifier");
	std::string Prefix = NameSpace ? std::string(NameSpace->getName()).append(".") : "";
	return Prefix.append(std::string(Name));
}
