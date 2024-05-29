//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGen.cpp - Code Generator implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CharUnits.h"
#include "CodeGen/CodeGenHeader.h"
#include "AST/ASTModule.h"
#include "AST/ASTNameSpace.h"
#include "Frontend/FrontendOptions.h"
#include "Basic/FileManager.h"
#include "Basic/TargetInfo.h"
#include "Basic/Debug.h"

#include <llvm/IR/LLVMContext.h>

using namespace fly;

/// toCharUnitsFromBits - Convert a size in bits to a size in characters.
CharUnits toCharUnitsFromBits(int64_t BitSize) {
    return CharUnits::fromQuantity(BitSize / 8);
}

CodeGen::CodeGen(DiagnosticsEngine &Diags, CodeGenOptions &CodeGenOpts,
                 const std::shared_ptr<TargetOptions> &TargetOpts,
                 BackendActionKind BackendAction, bool ShowTimers) :
        Diags(Diags), CodeGenOpts(CodeGenOpts), TargetOpts(*TargetOpts),
        Target(CreateTargetInfo(Diags, TargetOpts)), ActionKind(BackendAction),
        ShowTimers(ShowTimers) {

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
    Int8PtrPtrTy = Int8PtrTy->getPointerTo(0);

    PointerWidthInBits = Target->getPointerWidth(0);
    PointerAlignInBytes = toCharUnitsFromBits(Target->getPointerAlign(0)).getQuantity();
    SizeSizeInBytes = toCharUnitsFromBits(Target->getMaxPointerWidth()).getQuantity();
    IntAlignInBytes = toCharUnitsFromBits(Target->getIntAlign()).getQuantity();
    IntTy = llvm::IntegerType::get(LLVMCtx, Target->getIntWidth());
    IntPtrTy = llvm::IntegerType::get(LLVMCtx, Target->getMaxPointerWidth());
//    AllocaInt8PtrTy = Int8Ty->getPointerTo(Module->getDataLayout().getAllocaAddrSpace());
//
//    ErrorTy =

}

std::string CodeGen::getOutputFileName(llvm::StringRef BaseInput) {
    StringRef FileName = llvm::sys::path::filename(BaseInput);
    std::string Name = FileName.str();//.substr(0,FileName.size()-4/* sizeof('.fly') = 4 */).str();
    switch (ActionKind) {
        case Backend_EmitNothing:
            FLY_DEBUG_MESSAGE("CodeGen", "getOutputFileName","return ''");
            return "";
        case Backend_EmitLL:
            FLY_DEBUG_MESSAGE("CodeGen", "getOutputFileName","return " << Name + ".ll");
            return Name + ".ll";
        case Backend_EmitBC:
            FLY_DEBUG_MESSAGE("CodeGen", "getOutputFileName","return " << Name + ".bc");
            return Name + ".bc";
        case Backend_EmitAssembly:
            FLY_DEBUG_MESSAGE("CodeGen", "getOutputFileName","return " << Name + ".s");
            return Name + ".s";
        case Backend_EmitObj:
            FLY_DEBUG_MESSAGE("CodeGen", "getOutputFileName","return " << Name + ".o");
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
    FLY_DEBUG_MESSAGE("CodeGen", "Emit",
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
            std::make_unique<raw_fd_ostream>(OutName, ErrCode, llvm::sys::fs::F_None);

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

LLVMContext &CodeGen::getLLVMCtx() {
    return LLVMCtx;
}

std::vector<llvm::Module *> CodeGen::GenerateModules(ASTContext &AST) {
    FLY_DEBUG("CodeGen", "GenerateModules");
    std::vector<llvm::Module *> Modules;
    for (auto &Entry : AST.getModules()) {
        Diags.getClient()->BeginSourceFile();
        CodeGenModule *CGM = GenerateModule(*Entry.getValue());
        CGM->GenAll();
        Modules.push_back(CGM->Module);
        Diags.getClient()->EndSourceFile();
    }
    return Modules;
}

CodeGenModule *CodeGen::GenerateModule(ASTModule &AST) {
    FLY_DEBUG("CodeGen", "GenerateModule");
    CodeGenModule *CGM = new CodeGenModule(Diags, this, AST, LLVMCtx, *Target, CodeGenOpts);
    AST.setCodeGen(CGM);
    return CGM;
}

void CodeGen::GenerateHeaders(ASTContext &AST) {
    for (auto &Entry : AST.getModules()) {
        Diags.getClient()->BeginSourceFile();
        GenerateHeader(*Entry.getValue());
        Diags.getClient()->EndSourceFile();
    }
}

void CodeGen::GenerateHeader(ASTModule &Module){
    FLY_DEBUG("CodeGen", "GenerateHeader");
    return CodeGenHeader::CreateFile(Diags, CodeGenOpts, Module);
}

std::string CodeGen::toIdentifier(llvm::StringRef Name, llvm::StringRef NameSpace, llvm::StringRef ClassName) {
    std::string Prefix = NameSpace == "default" ? "" : std::string(NameSpace).append("_");
    return Prefix.append(ClassName.empty() ?
                std::string(Name) :
                std::string(ClassName).append("_").append(std::string(Name)));
}
