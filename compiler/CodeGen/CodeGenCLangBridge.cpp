//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenCLangBridge.cpp - Compile-time C FFI bridge codegen
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenExpr.h"

#include "Basic/CodeGenOptions.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGenVar.h"

#include <Sema/SemaCall.h>
#include <Sema/SemaFunctionBase.h>
#include <Sema/SemaClassAttribute.h>
#include <Sema/SemaClassType.h>
#include <Sema/SemaNode.h>
#include <Sema/SemaType.h>
#include <Sema/SemaValue.h>
#include <Sema/SemaVar.h>

#include <algorithm>
#include <string>

using namespace fly;

// ─── Library name normalisation ───────────────────────────────────────────────

std::string CodeGenExpr::NormalizeCLangLibFlag(const std::string &LibStr) {
    llvm::StringRef Lib(LibStr);

    // Absolute path → pass as-is (static or dynamic already located)
    if (Lib.starts_with("/"))
        return LibStr;

    // Explicit static archive → use -l:<name> (GNU ld exact-name syntax)
    if (Lib.ends_with(".a"))
        return "-l:" + LibStr;

    // Dynamic library: strip 'lib' prefix and extension (.so, .so.N, .dylib, .dll)
    std::string Name(Lib.str());
    auto Dot = Name.find('.');
    if (Dot != std::string::npos)
        Name = Name.substr(0, Dot);
    if (Name.size() >= 3 && Name.substr(0, 3) == "lib")
        Name = Name.substr(3);
    return "-l" + Name;
}

// ─── Bridge call codegen ──────────────────────────────────────────────────────

void CodeGenExpr::GenCLangBridgeCall(SemaCall *Sema) {
    std::string FnName(Sema->getFunction()->getName());

    // Args layout: (const string lib, const string sym, const Args args [, T out])
    auto &ArgExprs = Sema->getArgs();
    if (ArgExprs.size() < 3) { V = nullptr; return; }

    // ─── Extract lib and sym string literals (must be compile-time constants) ─

    SemaExpr *LibExpr = ArgExprs[0];
    SemaExpr *SymExpr = ArgExprs[1];

    auto isStringLiteral = [](SemaExpr *E) {
        return E->getKind() == SemaKind::VALUE &&
               E->getType() && E->getType()->getKind() == SemaKind::TYPE_STRING;
    };
    if (!isStringLiteral(LibExpr) || !isStringLiteral(SymExpr)) {
        V = nullptr;
        return;
    }
    std::string LibStr = static_cast<SemaStringValue *>(LibExpr)->getValue().str();
    std::string SymStr = static_cast<SemaStringValue *>(SymExpr)->getValue().str();

    // Register library flag with the linker (deduplicated)
    std::string LibFlag = NormalizeCLangLibFlag(LibStr);
    auto &LinkerOpts = CGM->CGOpts.LinkerOptions;
    if (std::find(LinkerOpts.begin(), LinkerOpts.end(), LibFlag) == LinkerOpts.end())
        LinkerOpts.push_back(LibFlag);

    // ─── C return type inferred from function name ─────────────────────────

    bool IsVoid = (FnName == "callVoid");
    bool IsPtr  = (FnName == "callPtr");
    bool IsBool = (FnName == "callBool");
    llvm::Type *CRetTy = nullptr;
    if (IsVoid)                        CRetTy = CodeGen::VoidTy;
    else if (IsPtr)                    CRetTy = llvm::PointerType::getUnqual(CGM->LLVMCtx);
    else if (FnName == "callInt")      CRetTy = CodeGen::Int32Ty;
    else if (FnName == "callLong")     CRetTy = CodeGen::Int64Ty;
    else if (FnName == "callFloat")    CRetTy = CodeGen::FloatTy;
    else if (FnName == "callDouble")   CRetTy = CodeGen::DoubleTy;
    else if (IsBool)                   CRetTy = CodeGen::Int32Ty;
    else { V = nullptr; return; }

    // ─── Build C argument list from concrete Args struct fields ────────────

    SemaExpr *ArgsExpr = ArgExprs[2];
    ArgsExpr->accept(*CGM);
    llvm::Value *ArgsPtr = ArgsExpr->getCodeGen()->getValue();

    SemaClassType *ArgsType = static_cast<SemaClassType *>(ArgsExpr->getType());
    llvm::StructType *ArgsStructTy = ArgsType->getCodeGen()->getType();

    llvm::SmallVector<llvm::Value *, 8> CArgs;
    llvm::SmallVector<llvm::Type *, 8>  CArgTys;

    for (SemaNode *Node : ArgsType->getNodes()) {
        if (Node->getKind() != SemaKind::ATTRIBUTE)
            continue;
        SemaClassAttribute *Attr = static_cast<SemaClassAttribute *>(Node);

        size_t Idx = Attr->getCodeGen()->getIndex();
        Attr->getType()->accept(*CGM);
        llvm::Type *FieldTy = Attr->getType()->getCodeGen()->getType();

        llvm::Value *FieldPtr = Builder->CreateInBoundsGEP(ArgsStructTy, ArgsPtr,
            {CodeGen::Zero, llvm::ConstantInt::get(CodeGen::Int32Ty, (uint32_t)Idx)});
        llvm::Value *FieldVal = Builder->CreateLoad(FieldTy, FieldPtr);

        llvm::Value *CArg;
        llvm::Type  *CArgTy;
        if (FieldTy == CodeGen::StringTy) {
            // string { ptr, i32 } → const char* (first element)
            CArg   = Builder->CreateExtractValue(FieldVal, 0);
            CArgTy = llvm::PointerType::getUnqual(CGM->LLVMCtx);
        } else if (FieldTy->isStructTy()) {
            // Ptr { i64 addr } → void* (extract addr → inttoptr)
            llvm::Value *Addr = Builder->CreateExtractValue(FieldVal, 0);
            CArg   = Builder->CreateIntToPtr(Addr, llvm::PointerType::getUnqual(CGM->LLVMCtx));
            CArgTy = llvm::PointerType::getUnqual(CGM->LLVMCtx);
        } else {
            CArg   = FieldVal;
            CArgTy = FieldTy;
        }
        CArgs.push_back(CArg);
        CArgTys.push_back(CArgTy);
    }

    // ─── Emit the C call ───────────────────────────────────────────────────

    llvm::FunctionType *FTy = llvm::FunctionType::get(CRetTy, CArgTys, false);
    llvm::FunctionCallee Callee = CGM->Module->getOrInsertFunction(SymStr, FTy);
    llvm::Value *CResult = Builder->CreateCall(Callee, CArgs);

    // ─── Write output parameter ────────────────────────────────────────────

    if (IsVoid) {
        V = CResult;
        return;
    }

    // Output is ArgExprs[3] for non-void variants
    if (ArgExprs.size() < 4) { V = nullptr; return; }
    SemaVar *OutVar = static_cast<SemaVar *>(ArgExprs[3]);

    if (IsPtr) {
        // Allocate Ptr struct memory on stack (variable's alloca holds null initially)
        SemaClassType *PtrType = static_cast<SemaClassType *>(OutVar->getType());
        llvm::StructType *PtrStructTy = PtrType->getCodeGen()->getType();
        llvm::Value *PtrMem = Builder->CreateAlloca(PtrStructTy);

        // Write struct pointer into the variable's alloca
        Builder->CreateStore(PtrMem, OutVar->getCodeGen()->getPointer());
        OutVar->getCodeGen()->resetLoad();

        // Store C pointer as i64 into Ptr.addr (field 0 — struct has no vtable)
        llvm::Value *Addr = Builder->CreatePtrToInt(CResult, CodeGen::Int64Ty);
        llvm::Value *AddrPtr = Builder->CreateInBoundsGEP(PtrStructTy, PtrMem,
            {CodeGen::Zero, llvm::ConstantInt::get(CodeGen::Int32Ty, 0)});
        Builder->CreateStore(Addr, AddrPtr);

        V = PtrMem;
    } else {
        llvm::Value *StoreVal = CResult;
        if (IsBool) {
            StoreVal = Builder->CreateTrunc(CResult, CodeGen::BoolTy);
        } else {
            llvm::Type *OutTy = OutVar->getCodeGen()->getType();
            if (CResult->getType() != OutTy &&
                CResult->getType()->isIntegerTy() && OutTy->isIntegerTy()) {
                unsigned Src = CResult->getType()->getIntegerBitWidth();
                unsigned Dst = OutTy->getIntegerBitWidth();
                StoreVal = (Src > Dst)
                    ? Builder->CreateTrunc(CResult, OutTy)
                    : Builder->CreateSExt(CResult, OutTy);
            }
        }
        OutVar->getCodeGen()->Store(StoreVal);
        OutVar->getCodeGen()->resetLoad();
        V = CResult;
    }
}
