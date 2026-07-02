//===-------------------------------------------------------------------------------------------------------------===//
// compiler/CodeGen/CodeGenModule.cpp - LLVM IR emission for modules
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
#include "Sema/SemaFunction.h"
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
#include <Sema/SemaEnumAccessor.h>
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
#include "Sema/SemaCaseStmt.h"
#include "Sema/SemaTestStmt.h"

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
    llvm::DIScope *Scope = DebugScopeStack.empty()
        ? (llvm::DIScope *)Fn->getSubprogram()
        : DebugScopeStack.back();
    Builder->SetCurrentDebugLocation(
        llvm::DILocation::get(LLVMCtx, Line, Col, Scope));
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
            case SemaIntTypeKind::TYPE_POINTER:
                DIT = DBuilder->createBasicType("pointer", 64, llvm::dwarf::DW_ATE_unsigned); break;
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
    } else if (Ty->isEnum()) {
        auto *ET = static_cast<SemaEnumType *>(Ty);
        llvm::SmallVector<llvm::Metadata *, 8> Enumerators;
        for (auto &[Name, Entry] : ET->getEntries())
            Enumerators.push_back(DBuilder->createEnumerator(
                Name.str(), (int64_t)Entry->getIndex()));
        llvm::DIType *UnderlyingTy =
            DBuilder->createBasicType("int", 32, llvm::dwarf::DW_ATE_signed);
        DIT = DBuilder->createEnumerationType(
            DebugCU, ET->getName(), DebugFile, /*LineNo=*/0,
            /*SizeInBits=*/32, /*AlignInBits=*/32,
            DBuilder->getOrCreateArray(Enumerators), UnderlyingTy);
    } else if (Ty->isArray()) {
        auto *AT = static_cast<SemaArrayType *>(Ty);
        llvm::DIType *ElemDI = GetOrCreateDIType(AT->getElementType());
        if (!ElemDI) ElemDI = DBuilder->createUnspecifiedType("?");
        DIT = DBuilder->createPointerType(ElemDI, PtrBits, 0,
                                          std::nullopt, Ty->getName());
    } else if (Ty->isClass()) {
        auto *CT = static_cast<SemaClassType *>(Ty);
        CodeGenClass *CGC = CT->getCodeGen();
        if (!CGC) {
            DIT = DBuilder->createUnspecifiedType(CT->getName());
        } else {
            // Forward declare to break recursive type references
            auto *Fwd = DBuilder->createReplaceableCompositeType(
                llvm::dwarf::DW_TAG_structure_type,
                CT->getName(), DebugCU, DebugFile, 0);
            DITypeCache[Ty] = Fwd;

            const llvm::StructLayout *Layout =
                Module->getDataLayout().getStructLayout(CGC->getType());
            llvm::SmallVector<llvm::Metadata *, 8> Members;

            // VTable field (CLASS and INTERFACE only — always index 0)
            if (CT->getClassKind() != SemaClassKind::STRUCT) {
                llvm::DIType *VTPtrTy =
                    DBuilder->createPointerType(nullptr, PtrBits);
                Members.push_back(DBuilder->createMemberType(
                    DebugCU, "__vtable", DebugFile, 0,
                    PtrBits, PtrBits,
                    Layout->getElementOffsetInBits(0),
                    llvm::DINode::FlagArtificial, VTPtrTy));
            }

            // Attributes — CodeGenVar::getIndex() gives the struct field index
            for (auto &[Name, Attr] : CT->getAttributes()) {
                size_t FieldIdx = Attr->getCodeGen()->getIndex();
                uint64_t OffsetBits = Layout->getElementOffsetInBits(FieldIdx);
                llvm::DIType *AttrDI = GetOrCreateDIType(Attr->getType());
                uint64_t SzBits = AttrDI ? AttrDI->getSizeInBits() : PtrBits;
                Members.push_back(DBuilder->createMemberType(
                    DebugCU, Name.str(), DebugFile, 0,
                    SzBits, SzBits, OffsetBits,
                    llvm::DINode::FlagZero, AttrDI));
            }

            uint64_t TotalBits = Layout->getSizeInBits();
            uint64_t AlignBits =
                Module->getDataLayout().getABITypeAlign(CGC->getType()).value() * 8;

            auto *StructDI = DBuilder->createStructType(
                DebugCU, CT->getName(), DebugFile, 0,
                TotalBits, AlignBits, llvm::DINode::FlagZero,
                /*DerivedFrom=*/nullptr,
                DBuilder->getOrCreateArray(Members));

            Fwd->replaceAllUsesWith(StructDI);
            llvm::MDNode::deleteTemporary(Fwd);
            DIT = StructDI;
        }
    } else {
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

void CodeGenModule::GenerateDeclarations(SemaModule &Sema) {
    FLY_DEBUG_SCOPE_MSG("CodeGenModule", "GenerateDeclarations", "Module: " + Sema.getName().str());
    CurrentSemaModule = &Sema;

    // Generate all nodes (functions, classes, enums). Function bodies are queued
    // in Functions and emitted later by GenerateBodies().
    for (auto &Node : Sema.getNodes()) {
        Node->accept(*this);
    }

    // Drain classes whose layout-dependent steps were deferred during a cyclic
    // build. All types reachable from this module's declarations are now fully
    // sized, so per-base vtables and init-constructor bodies can be emitted.
    // Index-based: FinishBuild may itself force-build (and defer) further classes.
    for (size_t i = 0; i < DeferredClassFinish.size(); ++i) {
        DeferredClassFinish[i]->FinishBuild();
    }
    DeferredClassFinish.clear();
}

void CodeGenModule::GenerateBodies() {
    FLY_DEBUG_SCOPE("CodeGenModule", "GenerateBodies");
    // Generate Function Bodies in a second pass
	for (auto &FB : Functions) {
		FB->accept(*this);
	}
}

void CodeGenModule::visit(SemaModule &Sema) {
    FLY_DEBUG_SCOPE_MSG("CodeGenModule", "visit(SemaModule)", "Module: " + Sema.getName().str());
    GenerateDeclarations(Sema);
    GenerateBodies();
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
	FLY_DEBUG_SCOPE_MSG("CodeGenModule", "visit(SemaClassType)", "Class: " + Sema.getAST().getName().str());

	// Suites get their own codegen path: no vtable, emit implicit main()
	if (Sema.getClassKind() == SemaClassKind::SUITE) {
		EmitSuite(Sema);
		return;
	}

	if (Sema.getCodeGen() == nullptr) {
		// Generic template classes have no concrete code to generate.
		if (Sema.isGeneric() && Sema.getGenericTemplate() == nullptr)
			return;
		// Specializations are always generated locally in the using module, even when
		// the generic template lives in an external library.
		// External = defined in an imported library, i.e. not one of the input
		// files compiled into this module. (Checking != CurrentSemaModule alone is
		// wrong when several files are compiled together: a class first referenced
		// from a sibling file would be misclassified as external.)
		bool isExternal = (Sema.getGenericTemplate() == nullptr) &&
		                  !ownsModule(&Sema.getModule());
		// Two-phase construction: Phase 1 creates the LLVM struct type (constructor),
		// then setCodeGen() is called so self-referential return type lookups during
		// Phase 2 (Build()) find the already-constructed CodeGen instead of recursing.
		CodeGenClass *CGC = new CodeGenClass(this, &Sema, isExternal);
		Sema.setCodeGen(CGC);
		CGC->Build();
	}
}

void CodeGenModule::visit(SemaClassMethod &Sema) {
	FLY_DEBUG_SCOPE_MSG("CodeGenModule", "visit(SemaClassMethod)", "Method: " + Sema.getName().str());
	CurrentFunction = &Sema;
	if (Sema.getCodeGen()) {
		Sema.getCodeGen()->GenBody();
	}
}

void CodeGenModule::visit(SemaFunction &Sema) {
	FLY_DEBUG_SCOPE_MSG("CodeGenModule", "visit(SemaFunction)", "Function: " + Sema.getName().str());
	// Generic template functions are never code-generated directly;
	// only their concrete specializations (which have no TypeParams) produce LLVM IR.
	if (Sema.isGeneric())
		return;
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
	FLY_DEBUG_SCOPE("CodeGenModule", "visit(SemaClassAttribute)");
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
			SemaClassType &DeclClass = Sema.getClass();
			llvm::Value *Instance = Method->getThis()->getCodeGen()->getValue();
			// For an INHERITED field, navigate the FULL base-subobject chain from the
			// current `this` to the declaring class (recursively, for grandparent+ fields)
			// instead of GEP-ing the declaring struct directly on the derived pointer.
			if (Method->getClass() && Method->getClass() != &DeclClass) {
				llvm::Value *Adj = Method->getClass()->getCodeGen()->getBaseInstance(Instance, &DeclClass);
				if (Adj) Instance = Adj;
			}
			llvm::StructType *StructTy = DeclClass.getCodeGen()->getType();
			size_t FieldIdx = Sema.getCodeGen()->getIndex();
			llvm::Value *FieldPtr = Builder->CreateInBoundsGEP(StructTy, Instance,
				{CodeGen::Zero, llvm::ConstantInt::get(CodeGen::Int32Ty, FieldIdx)});
			Sema.getCodeGen()->setPointer(FieldPtr);
		}
	}
}

void CodeGenModule::visit(SemaLocalVar &Sema) {
	FLY_DEBUG_SCOPE("CodeGenModule", "visit(SemaLocalVar)");
	if (Sema.getCodeGen() == nullptr) {
		Sema.getType()->accept(*this);
		llvm::Type *T = Sema.getType()->getCodeGen()->getType();
		CodeGenVar *CGV = new CodeGenVar(this, &Sema, T);
		Sema.setCodeGen(CGV);
	}
}

void CodeGenModule::visit(SemaParam &Sema) {
	FLY_DEBUG_SCOPE("CodeGenModule", "visit(SemaParam)");
	if (Sema.getCodeGen() == nullptr) {
		Sema.getType()->accept(*this);
		llvm::Type *T = Sema.getType()->getCodeGen()->getType();
		CodeGenVar *CGV = new CodeGenVar(this, &Sema, T);
		Sema.setCodeGen(CGV);
	}
}

void CodeGenModule::visit(SemaClassInstance &Sema) {
	FLY_DEBUG_SCOPE("CodeGenModule", "visit(SemaClassInstance)");
	if (Sema.getCodeGen() == nullptr) {
		Sema.getType()->accept(*this);
		llvm::Type *T = Sema.getType()->getCodeGen()->getType();
		CodeGenVar *CGV = new CodeGenVar(this, &Sema, T);
		Sema.setCodeGen(CGV);
	}
}

void CodeGenModule::visit(SemaError &Sema) {
	FLY_DEBUG_SCOPE("CodeGenModule", "visit(SemaError)");
	if (Sema.getCodeGen() == nullptr) {
		Sema.getType()->accept(*this);
		llvm::Value *ErrorHandler = Builder->CreateAlloca(CG.ErrorPtrTy);
		CodeGenError *CGE = new CodeGenError(this, &Sema, ErrorHandler);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaMember &Sema) {
	FLY_DEBUG_SCOPE("CodeGenModule", "visit(SemaMember)");
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
	FLY_DEBUG_SCOPE("CodeGenModule", "visit(SemaCall)");
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaUnary &Sema) {
	FLY_DEBUG_SCOPE("CodeGenModule", "visit(SemaUnary)");
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaBinary &Sema) {
	FLY_DEBUG_SCOPE("CodeGenModule", "visit(SemaBinary)");
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaTernary &Sema) {
	FLY_DEBUG_SCOPE("CodeGenModule", "visit(SemaTernary)");
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaCast &Sema) {
	FLY_DEBUG_SCOPE("CodeGenModule", "visit(SemaCast)");
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

void CodeGenModule::visit(SemaEnumAccessor &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
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
	FLY_DEBUG_SCOPE("CodeGenModule", "visit(SemaBlockStmt)");
	AllocCleanupStack.push_back(&Sema);

	if (DBuilder && DebugFile) {
		llvm::DIScope *Parent;
		if (!DebugScopeStack.empty()) {
			Parent = DebugScopeStack.back();
		} else {
			llvm::BasicBlock *BB = Builder->GetInsertBlock();
			llvm::Function *Fn = BB ? BB->getParent() : nullptr;
			Parent = (Fn && Fn->getSubprogram())
			             ? (llvm::DIScope *)Fn->getSubprogram()
			             : (llvm::DIScope *)DebugCU;
		}
		unsigned Line = 0;
		if (SM && !Sema.getContent().empty()) {
			const SourceLocation &Loc = Sema.getContent().front()->getAST()->getLocation();
			if (Loc.isValid()) Line = SM->getSpellingLineNumber(Loc);
		}
		DebugScopeStack.push_back(
		    DBuilder->createLexicalBlock(Parent, DebugFile, Line, 0));
	}

	for (SemaStmt *Stmt : Sema.getContent()) {
		Stmt->accept(*this);
	}

	if (DBuilder && !DebugScopeStack.empty())
		DebugScopeStack.pop_back();

	// Emit this scope's cleanup only on a normal (fall-through) exit. If the block
	// already ended with a terminator (a `return`/`break`/`continue` emitted its own
	// cleanup for all enclosing frames), appending frees here would place instructions
	// after the terminator → "Terminator found in the middle of a basic block".
	llvm::BasicBlock *CurBB = Builder->GetInsertBlock();
	if (!CurBB || !CurBB->getTerminator())
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

// Returns (or creates) the thread-local i8* sentinel used by the test system.
// In production modules this is declared as an external reference.
// In suite modules it is defined and initialized to null.
static llvm::GlobalVariable *GetOrCreateTestCtxPtr(llvm::Module *M, llvm::LLVMContext &Ctx,
                                                    bool IsDefinition) {
    const char *Name = "__fly_test_ctx_ptr";
    if (auto *Existing = M->getNamedGlobal(Name))
        return Existing;

    auto *PtrTy = llvm::PointerType::getUnqual(Ctx);
    auto *GV = new llvm::GlobalVariable(
        *M, PtrTy,
        /*isConstant=*/false,
        IsDefinition ? llvm::GlobalValue::ExternalLinkage : llvm::GlobalValue::ExternalLinkage,
        IsDefinition ? llvm::ConstantPointerNull::get(PtrTy) : nullptr,
        Name);
    GV->setThreadLocal(true);
    return GV;
}

// Classify a suite method name into its role
static bool isSuiteSetup(llvm::StringRef N)    { return N == "setup"; }
static bool isSuiteTeardown(llvm::StringRef N) { return N == "teardown"; }
static bool isSuiteTestMethod(llvm::StringRef N) { return N.ends_with("Test"); }

void CodeGenModule::EmitSuite(SemaClassType &Sema) {
    FLY_DEBUG_SCOPE_MSG("CodeGenModule", "EmitSuite",
                        "Suite: " + Sema.getAST().getName().str());

    // Emit the TLS context pointer as a definition in this module
    GetOrCreateTestCtxPtr(Module, LLVMCtx, /*IsDefinition=*/true);

    // Classify suite methods by name convention
    SemaClassMethod *SetupM    = nullptr;
    SemaClassMethod *TeardownM = nullptr;
    llvm::SmallVector<SemaClassMethod *, 8> TestMethods;

    for (auto &[Name, M] : Sema.getMethods()) {
        if (isSuiteSetup(Name))        SetupM = M;
        else if (isSuiteTeardown(Name)) TeardownM = M;
        else if (isSuiteTestMethod(Name)) TestMethods.push_back(M);
    }

    // Suite methods are SemaClassMethod but CodeGenClass::Build() is never called for SUITE.
    // We must create CodeGenClassMethod instances here so the methods get LLVM functions and
    // their bodies are generated in the second pass (Functions vector).
    // A minimal empty struct type stands in for the suite "this" pointer.
    auto *SuiteStructTy = llvm::StructType::create(LLVMCtx,
                              "suite." + Sema.getAST().getName().str());
    SuiteStructTy->setBody({});  // empty — suite methods never dereference 'this'

    auto EnsureCompiled = [&](SemaClassMethod *M) {
        if (!M || M->getCodeGen()) return;
        auto *CGM = new CodeGenClassMethod(this, M, SuiteStructTy, 0);
        M->setCodeGen(CGM);
        Functions.push_back(M);
    };

    EnsureCompiled(SetupM);
    EnsureCompiled(TeardownM);
    for (auto *M : TestMethods) EnsureCompiled(M);

    // Helper to get the LLVM function from a SemaClassMethod
    auto GetFn = [](SemaClassMethod *M) -> llvm::Function * {
        if (!M || !M->getCodeGen()) return nullptr;
        return M->getCodeGen()->getFunction();
    };

    // Build implicit main(): int main() { setup(); test1(); ...; teardown(); return 0; }
    auto *Int32Ty = llvm::Type::getInt32Ty(LLVMCtx);
    auto *PtrTy   = llvm::PointerType::getUnqual(LLVMCtx);
    auto *NullPtr = llvm::ConstantPointerNull::get(PtrTy);
    auto *MainTy  = llvm::FunctionType::get(Int32Ty, /*isVarArg=*/false);
    auto *MainFn  = llvm::Function::Create(MainTy, llvm::GlobalValue::ExternalLinkage, "main", Module);
    auto *EntryBB = llvm::BasicBlock::Create(LLVMCtx, "entry", MainFn);
    Builder->SetInsertPoint(EntryBB);

    // Allocate and initialise a shared error handler for this suite's main().
    // Each test-method case receives its own fresh alloca (see visit(SemaCaseStmt));
    // setup/teardown/helpers receive this one as their first argument.
    llvm::Value *ErrAlloca = Builder->CreateAlloca(PtrTy, nullptr, "suite_err");
    Builder->CreateStore(NullPtr, ErrAlloca);

    llvm::GlobalVariable *TLSPtr = GetOrCreateTestCtxPtr(Module, LLVMCtx, /*IsDefinition=*/false);
    // Set TLS ptr to a non-null sentinel (address 1) before tests run
    auto *Sentinel = llvm::ConstantExpr::getIntToPtr(
        llvm::ConstantInt::get(llvm::Type::getInt64Ty(LLVMCtx), 1), PtrTy);
    Builder->CreateStore(Sentinel, TLSPtr);

    // Helper: call a suite method passing the error handler as arg 0, null for this + extras
    auto CallMethod = [&](SemaClassMethod *M) {
        auto *Fn = GetFn(M);
        if (!Fn) return;
        llvm::SmallVector<llvm::Value *, 4> Args;
        size_t numParams = Fn->getFunctionType()->getNumParams();
        if (numParams >= 1) Args.push_back(ErrAlloca); // error handler
        for (size_t i = 1; i < numParams; ++i) Args.push_back(NullPtr); // this + extras
        Builder->CreateCall(Fn->getFunctionType(), Fn, Args);
    };

    CallMethod(SetupM);
    for (auto *M : TestMethods) CallMethod(M);
    CallMethod(TeardownM);

    // Clear TLS ptr after all tests
    Builder->CreateStore(NullPtr, TLSPtr);
    Builder->CreateRet(llvm::ConstantInt::get(Int32Ty, 0));
}

void CodeGenModule::visit(SemaTestStmt &Sema) {
    FLY_DEBUG_SCOPE("CodeGenModule", "visit(SemaTestStmt)");

    if (!CGOpts.TestMode) return;

    llvm::Function *Fn = CurrentFunction ? CurrentFunction->getCodeGen()->getFunction() : nullptr;
    if (!Fn) return;

    llvm::GlobalVariable *TLSPtr = GetOrCreateTestCtxPtr(Module, LLVMCtx, /*IsDefinition=*/false);

    // Load the TLS pointer
    auto *PtrTy = llvm::PointerType::getUnqual(LLVMCtx);
    llvm::Value *CtxVal = Builder->CreateLoad(PtrTy, TLSPtr, "test_ctx");

    // Branch: if null → skip, else → body
    llvm::BasicBlock *BodyBB  = llvm::BasicBlock::Create(LLVMCtx, "testbody",  Fn);
    llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(LLVMCtx, "testmerge", Fn);

    llvm::Value *IsNull = Builder->CreateIsNull(CtxVal, "ctx_isnull");
    Builder->CreateCondBr(IsNull, MergeBB, BodyBB);

    // Emit body
    Builder->SetInsertPoint(BodyBB);
    Sema.getBody()->accept(*this);
    if (!Builder->GetInsertBlock()->getTerminator())
        Builder->CreateBr(MergeBB);

    Builder->SetInsertPoint(MergeBB);
}

void CodeGenModule::visit(SemaCaseStmt &Sema) {
    FLY_DEBUG_SCOPE("CodeGenModule", "visit(SemaCaseStmt)");

    llvm::Function *Fn = CurrentFunction ? CurrentFunction->getCodeGen()->getFunction() : nullptr;
    if (!Fn) return;

    // Emit as a labelled basic block; falls through to the next statement
    llvm::BasicBlock *CaseBB = llvm::BasicBlock::Create(LLVMCtx, "case." + Sema.getLabel(), Fn);

    if (!Builder->GetInsertBlock()->getTerminator())
        Builder->CreateBr(CaseBB);

    Builder->SetInsertPoint(CaseBB);

    // Each case gets a fresh, isolated error handler so assertions are independent.
    auto *PtrTy   = llvm::PointerType::getUnqual(LLVMCtx);
    auto *NullPtr = llvm::ConstantPointerNull::get(PtrTy);
    llvm::Value *CaseErrPtr = Builder->CreateAlloca(PtrTy, nullptr,
                                                    "case_err." + Sema.getLabel());
    Builder->CreateStore(NullPtr, CaseErrPtr);

    CodeGenError *SavedErr = CurrentErrorHandler;
    auto *CaseErrCG = new CodeGenError(this, nullptr, CaseErrPtr);
    CurrentErrorHandler = CaseErrCG;

    Sema.getBody()->accept(*this);

    CurrentErrorHandler = SavedErr;
    delete CaseErrCG;
}
