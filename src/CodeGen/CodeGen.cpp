//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGen.cpp - Code Generator implementation
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
}

std::string CodeGen::getOutputFileName(llvm::StringRef BaseInput) {
    StringRef FileName = llvm::sys::path::filename(BaseInput);
    std::string Name = FileName.str();//.substr(0,FileName.size()-4/* sizeof('.fly') = 4 */).str();
    switch (ActionKind) {
        case Backend_EmitNothing:
            FLY_DEBUG_START_MSG("CodeGen", "getOutputFileName","return ''");
            return "";
        case Backend_EmitLL:
            FLY_DEBUG_START_MSG("CodeGen", "getOutputFileName","return " << Name + ".ll");
            return Name + ".ll";
        case Backend_EmitBC:
            FLY_DEBUG_START_MSG("CodeGen", "getOutputFileName","return " << Name + ".bc");
            return Name + ".bc";
        case Backend_EmitAssembly:
            FLY_DEBUG_START_MSG("CodeGen", "getOutputFileName","return " << Name + ".s");
            return Name + ".s";
        case Backend_EmitObj:
            FLY_DEBUG_START_MSG("CodeGen", "getOutputFileName","return " << Name + ".o");
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
    FLY_DEBUG_START_MSG("CodeGen", "Emit",
                      "Module.Name=" << M->getName() << "\nModule.Output=" << str(M));

    std::string OutputFileName = OutName.empty() ? getOutputFileName(M->getName()) : OutName.str();
    // TODO link to libraries
//    Link = new llvm::Linker(*OutModule);
//    Linker::linkModules(*OutModule, std::move(M));

    // Skip CodeGenModule instance creation
    if (ActionKind == Backend_EmitNothing) {
        return;
    }

    std::error_code ErrCode;
    std::unique_ptr<llvm::raw_fd_ostream> OS =
            std::make_unique<llvm::raw_fd_ostream>(OutName, ErrCode, llvm::sys::fs::F_None);

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

llvm::SmallVector<llvm::Module *, 8> CodeGen::GenerateModules(llvm::SmallVector<SemaModule *, 8> &SemaModules) {
    FLY_DEBUG_START("CodeGen", "GenerateModules");

	llvm::SmallVector<llvm::Module *, 8> Modules;
	llvm::SmallVector<CodeGenModule *, 8> CodeGenModules;
    for (auto &Sema : SemaModules) {
        Diags.getClient()->BeginSourceFile();
    	CodeGenModule *CGM = new CodeGenModule(*this, Diags, Sema->getName(), LLVMCtx, *Target, CodeGenOpts);
    	Sema->accept(*CGM);
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

	FLY_DEBUG_END("CodeGen", "GenerateModules");
    return Modules;
}
