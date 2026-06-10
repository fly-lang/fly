//===--------------------------------------------------------------------------------------------------------------===//
// compiler/CodeGen/CodeGen.cpp - top-level code generator
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGen.h"

#include "AST/ASTModule.h"
#include "AST/ASTNameSpace.h"
#include "Basic/Debug.h"
#include "Basic/FileManager.h"
#include "Basic/TargetInfo.h"
#include "CodeGen/CharUnits.h"
#include "CodeGen/CodeGenHeader.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGenError.h"
#include "Frontend/FrontendOptions.h"
#include "Frontend/InputFile.h"
#include "Sema/SemaModule.h"
#include "Sema/SemaNameSpace.h"

#include <Sema/SymbolTable.h>
#include <llvm/IR/LLVMContext.h>


using namespace fly;

/// toCharUnitsFromBits - Convert a size in bits to a size in characters.
CharUnits toCharUnitsFromBits(int64_t BitSize) {
    return CharUnits::fromQuantity(BitSize / 8);
}
// Define static members
llvm::Type *CodeGen::VoidTy = nullptr;
llvm::IntegerType *CodeGen::BoolTy = nullptr;
llvm::IntegerType *CodeGen::Int8Ty = nullptr;
llvm::IntegerType *CodeGen::Int16Ty = nullptr;
llvm::IntegerType *CodeGen::Int32Ty = nullptr;
llvm::IntegerType *CodeGen::Int64Ty = nullptr;
llvm::Type *CodeGen::HalfTy = nullptr;
llvm::Type *CodeGen::BFloatTy = nullptr;
llvm::Type *CodeGen::FloatTy = nullptr;
llvm::Type *CodeGen::DoubleTy = nullptr;
llvm::IntegerType *CodeGen::IntTy = nullptr;
llvm::IntegerType *CodeGen::IntPtrTy = nullptr;
llvm::IntegerType *CodeGen::SizeTy = nullptr;
llvm::IntegerType *CodeGen::PtrDiffTy = nullptr;
llvm::PointerType *CodeGen::VoidPtrTy = nullptr;
llvm::PointerType *CodeGen::Int8PtrTy = nullptr;
llvm::PointerType *CodeGen::VoidPtrPtrTy = nullptr;
llvm::PointerType *CodeGen::Int8PtrPtrTy = nullptr;
llvm::PointerType *CodeGen::AllocaVoidPtrTy = nullptr;
llvm::PointerType *CodeGen::AllocaInt8PtrTy = nullptr;
unsigned char CodeGen::PointerWidthInBits = 0;
unsigned char CodeGen::PointerAlignInBytes = 0;
unsigned char CodeGen::PointerSizeInBytes = 0;
unsigned char CodeGen::SizeSizeInBytes = 0;
unsigned char CodeGen::SizeAlignInBytes = 0;
unsigned char CodeGen::IntSizeInBytes = 0;
unsigned char CodeGen::IntAlignInBytes = 0;
llvm::StructType *CodeGen::ErrorTy = nullptr;
llvm::PointerType *CodeGen::ErrorPtrTy = nullptr;
llvm::StructType *CodeGen::ArrayTy = nullptr;
llvm::PointerType *CodeGen::ArrayPtrTy = nullptr;
llvm::StructType *CodeGen::StringTy = nullptr;
llvm::PointerType *CodeGen::StringPtrTy = nullptr;
llvm::StructType *CodeGen::ComplexTy = nullptr;
llvm::ConstantInt *CodeGen::Zero = nullptr;

CodeGen::CodeGen(DiagnosticsEngine &Diags,
                 llvm::LLVMContext &LLVMCtx,
                 CodeGenOptions &CodeGenOpts,
                 const std::shared_ptr<TargetOptions> &TargetOpts,
                 BackendActionKind BackendAction, bool ShowTimers) :
        Diags(Diags),
        LLVMCtx(LLVMCtx),  // Store reference
        CodeGenOpts(CodeGenOpts),
        TargetOpts(*TargetOpts),
        Target(CreateTargetInfo(Diags, TargetOpts)),
        ActionKind(BackendAction),
        ShowTimers(ShowTimers) {

    // Always (re)initialize static types with the current LLVMContext
    // This is important for testing where context may be recreated per test
    InitializeTypes(LLVMCtx, *Target);
}

void CodeGen::InitializeTypes(llvm::LLVMContext &LLVMCtx, TargetInfo &Target) {
    // Configure Types
    VoidTy = llvm::Type::getVoidTy(LLVMCtx);
    BoolTy = llvm::Type::getInt1Ty(LLVMCtx);
    Int8Ty = llvm::Type::getInt8Ty(LLVMCtx);
    Int16Ty = llvm::Type::getInt16Ty(LLVMCtx);
    Int32Ty = llvm::Type::getInt32Ty(LLVMCtx);
    Int64Ty = llvm::Type::getInt64Ty(LLVMCtx);
    HalfTy = llvm::Type::getHalfTy(LLVMCtx);
    BFloatTy = llvm::Type::getBFloatTy(LLVMCtx);
    FloatTy = llvm::Type::getFloatTy(LLVMCtx);
    DoubleTy = llvm::Type::getDoubleTy(LLVMCtx);

    Int8PtrTy = Int8Ty->getPointerTo(0);
    VoidPtrTy = Int8PtrTy;  // void* is same as i8*
    Int8PtrPtrTy = Int8PtrTy->getPointerTo(0);
    VoidPtrPtrTy = Int8PtrPtrTy;  // void** is same as i8**

    // Configure platform-specific types
    PointerWidthInBits = Target.getPointerWidth(0);
    PointerAlignInBytes = toCharUnitsFromBits(Target.getPointerAlign(0)).getQuantity();
    PointerSizeInBytes = PointerAlignInBytes;
    SizeSizeInBytes = toCharUnitsFromBits(Target.getMaxPointerWidth()).getQuantity();
    SizeAlignInBytes = SizeSizeInBytes;
    IntAlignInBytes = toCharUnitsFromBits(Target.getIntAlign()).getQuantity();
    IntSizeInBytes = IntAlignInBytes;
    IntTy = llvm::IntegerType::get(LLVMCtx, Target.getIntWidth());
    IntPtrTy = llvm::IntegerType::get(LLVMCtx, Target.getMaxPointerWidth());
    SizeTy = IntPtrTy;  // size_t is same as intptr_t
    PtrDiffTy = IntPtrTy;  // ptrdiff_t is same as intptr_t

    Zero = llvm::ConstantInt::get(Int32Ty, 0);

    ErrorTy = CodeGenError::GenErrorType(LLVMCtx);
    ErrorPtrTy = llvm::PointerType::get(ErrorTy, 0);

    // Create Array structure type: { i8* data, size_t size }
    // Uses SizeTy which adapts to the target's pointer/size width
    llvm::SmallVector<llvm::Type *, 3> ArrayFields;
    ArrayFields.push_back(Int8PtrTy);     // i8* data
    ArrayFields.push_back(IntTy);     // size_t size (dimensions array)
    ArrayTy = llvm::StructType::create(LLVMCtx, ArrayFields, "array");
    ArrayPtrTy = llvm::PointerType::get(ArrayTy, 0);

    // Create String structure type: struct string { ptr byte*, size_t size }
    llvm::SmallVector<llvm::Type *, 2> StringFields;
    StringFields.push_back(Int8PtrTy);  // byte* ptr
    StringFields.push_back(IntTy);    // size_t size (dimensions string)
    StringTy = llvm::StructType::create(LLVMCtx, StringFields, "string");
    StringPtrTy = llvm::PointerType::get(StringTy, 0);

    // Create Complex structure type: struct complex { double real, double imag }
    llvm::SmallVector<llvm::Type *, 2> ComplexFields;
    ComplexFields.push_back(DoubleTy);  // double real
    ComplexFields.push_back(DoubleTy);  // double imag
    ComplexTy = llvm::StructType::create(LLVMCtx, ComplexFields, "complex");
}

std::string CodeGen::getOutputFileName(llvm::StringRef BaseInput) {
    FLY_DEBUG_SCOPE("CodeGen", "getOutputFileName");
    StringRef FileName = llvm::sys::path::filename(BaseInput);
    std::string Name = FileName.str();//.substr(0,FileName.size()-4/* sizeof('.fly') = 4 */).str();
    switch (ActionKind) {
        case Backend_EmitNothing:
            FLY_DEBUG_MSG("return ''");
            return "";
        case Backend_EmitLL:
            FLY_DEBUG_MSG("return " << Name + ".ll");
            return Name + ".ll";
        case Backend_EmitBC:
            FLY_DEBUG_MSG("return " << Name + ".bc");
            return Name + ".bc";
        case Backend_EmitAssembly:
            FLY_DEBUG_MSG("return " << Name + ".s");
            return Name + ".s";
        case Backend_EmitObj:
            FLY_DEBUG_MSG("return " << Name + ".o");
            return Name + ".o";
    }

    llvm_unreachable("Invalid backend action!");
}

std::string str(llvm::Module *M) {
    std::string str;
    llvm::raw_string_ostream rso(str);
    M->print(rso, nullptr);
    return str;
}

void CodeGen::Emit(llvm::Module *M, llvm::StringRef OutName) {
    FLY_DEBUG_SCOPE_MSG("CodeGen", "Emit",
                      "Module.Name=" << M->getName() << "\nModule.Output=" << str(M));

    std::string OutputFileName = OutName.empty() ? getOutputFileName(M->getName()) : OutName.str();

    // Skip CodeGenModule instance creation
    if (ActionKind == Backend_EmitNothing) {
        return;
    }

    std::error_code ErrCode;
    std::unique_ptr<llvm::raw_fd_ostream> OS =
            std::make_unique<llvm::raw_fd_ostream>(OutputFileName, ErrCode, llvm::sys::fs::OF_None);

    // Include Bitcode in module
    EmbedBitcode(M, CodeGenOpts, llvm::MemoryBufferRef());

    EmitBackendOutput(Diags, CodeGenOpts, TargetOpts, ShowTimers, Target->getDataLayout(),M, ActionKind, std::move(OS));
}

TargetInfo* CodeGen::CreateTargetInfo(DiagnosticsEngine &Diags,
                                      const std::shared_ptr<TargetOptions> &TargetOpts) {
    return TargetInfo::CreateTargetInfo(Diags, TargetOpts);
}

TargetInfo &CodeGen::getTargetInfo() const {
    assert(Target && "Compiler invocation has no target info!");
    return *Target;
}

llvm::LLVMContext &CodeGen::getLLVMCtx() {
    return LLVMCtx;
}

llvm::SmallVector<llvm::Module *, 8> CodeGen::GenerateModules(llvm::SmallVector<SemaModule *, 8> &SemaModules,
                                                             bool SingleModule) {
    FLY_DEBUG_SCOPE("CodeGen", "GenerateModules");

	llvm::SmallVector<llvm::Module *, 8> Modules;
	if (SemaModules.empty())
		return Modules;

	// ── Single combined module ──────────────────────────────────────────────
	// All input files are lowered into ONE llvm::Module so that references across
	// files (functions/classes/enums of the same namespace) resolve intra-module.
	// Emitting one module per file would leave a call to a function defined in a
	// sibling file as a bodyless stub → "Broken function". Used when a single
	// linked output (-o lib/shared/exe) is requested.
	if (SingleModule) {
		// First source filename is the LLVM module identifier (drives
		// getOutputFileName when no explicit -o is given).
		llvm::StringRef ModuleId = SemaModules[0]->getAST().getFile()->getFileName();
		CodeGenModule *CGM = new CodeGenModule(*this, Diags, ModuleId, LLVMCtx, *Target, CodeGenOpts, SM_);

		// Mark every input module as owned (local) so classes referenced across the
		// input files are not misclassified as external (declaration-only).
		for (auto &Sema : SemaModules)
			CGM->addOwnedModule(Sema);

		// Phase 1: declare every module's nodes (queues function bodies).
		for (auto &Sema : SemaModules) {
			Diags.getClient()->BeginSourceFile();
			CGM->GenerateDeclarations(*Sema);
			Diags.getClient()->EndSourceFile();
		}

		// Phase 2: generate all queued function bodies, now that every declaration exists.
		CGM->GenerateBodies();
		CGM->FinalizeDebugInfo();

		llvm::Module *M = CGM->getModule();
		CGM->Module = nullptr; // Transfer ownership; CGM no longer owns the Module
		Modules.push_back(M);
		delete CGM;
		return Modules;
	}

	// ── One module per input file (default) ─────────────────────────────────
	// Each file is emitted to its own .ll/.bc/.s/.o; cross-file symbols are
	// resolved later by the linker.
	llvm::SmallVector<CodeGenModule *, 8> CodeGenModules;
    for (auto &Sema : SemaModules) {
        Diags.getClient()->BeginSourceFile();
    	// Use the source filename (e.g. "main.fly") as the LLVM module identifier so that
    	// getOutputFileName("main.fly") produces "main.fly.ll" / "main.fly.o" etc.
    	llvm::StringRef ModuleId = Sema->getAST().getFile()->getFileName();
    	CodeGenModule *CGM = new CodeGenModule(*this, Diags, ModuleId, LLVMCtx, *Target, CodeGenOpts, SM_);
    	CGM->addOwnedModule(Sema);
    	Sema->accept(*CGM);
        CGM->FinalizeDebugInfo();
        Diags.getClient()->EndSourceFile();
    	// Transfer ownership: get the Module pointer and null it out in CodeGenModule
    	llvm::Module *M = CGM->getModule();
    	CGM->Module = nullptr; // Now CodeGenModule no longer owns this Module
    	Modules.push_back(M);
    	CodeGenModules.push_back(CGM);
    }

	// Clean up CodeGenModule objects (they no longer own the Modules)
	for (auto *CGM : CodeGenModules) {
		delete CGM;
	}

    return Modules;
}
