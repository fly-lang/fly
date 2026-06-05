//===--------------------------------------------------------------------------------------------------------------===//
// compiler/CodeGen/CodeGenStdLibCLang.cpp - fly.bridge.CLang code generation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenStdLibCLang.h"

#include "Basic/CodeGenOptions.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGenVar.h"

#include <Sema/SemaCall.h>
#include <Sema/SemaFunctionBase.h>
#include <Sema/SemaClassAttribute.h>
#include <Sema/SemaClassMethod.h>
#include <Sema/SemaClassType.h>
#include <Sema/SemaNode.h>
#include <Sema/SemaType.h>
#include <Sema/SemaValue.h>
#include <Sema/SemaVar.h>

#include <algorithm>
#include <string>

using namespace fly;

CodeGenStdLibCLang::CodeGenStdLibCLang(CodeGenModule *CGM, llvm::IRBuilder<> *Builder, llvm::Value *&V)
    : CGM(CGM), Builder(Builder), V(V) {}

// ─── Library name normalisation ───────────────────────────────────────────────

std::string CodeGenStdLibCLang::NormalizeCLangLibFlag(const std::string &LibStr) {
    llvm::StringRef Lib(LibStr);

    // Absolute path → pass as-is
    if (Lib.starts_with("/"))
        return LibStr;

    // Explicit static archive → -l:<name>
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

// ─── String literal check ─────────────────────────────────────────────────────

static bool isStringLiteral(SemaExpr *E) {
    return E->getKind() == SemaKind::VALUE &&
           E->getType() && E->getType()->getKind() == SemaKind::TYPE_STRING;
}

// ─── Constructor capture ──────────────────────────────────────────────────────
//
// Called after the normal constructor path has allocated the CLang instance.
// Captures lib literal → registers linker flag → stores in CLangLibMap.

void CodeGenStdLibCLang::GenConstructorCapture(SemaCall *Sema, llvm::Value *InstancePtr) {
    auto &ArgExprs = Sema->getArgs();

    if (ArgExprs.empty() || !isStringLiteral(ArgExprs[0]))
        return;

    std::string LibStr = static_cast<SemaStringValue *>(ArgExprs[0])->getValue().str();

    // Register linker flag (deduplicated)
    std::string LibFlag = NormalizeCLangLibFlag(LibStr);
    auto &LinkerOpts = CGM->CGOpts.LinkerOptions;
    if (std::find(LinkerOpts.begin(), LinkerOpts.end(), LibFlag) == LinkerOpts.end())
        LinkerOpts.push_back(LibFlag);

    // Associate this instance pointer with the lib string for later call() use
    CGM->CLangLibMap[InstancePtr] = LibStr;
}

// ─── Args struct → C argument list ───────────────────────────────────────────

void CodeGenStdLibCLang::BuildCArgsFromArgsStruct(
        SemaExpr *ArgsExpr,
        llvm::SmallVector<llvm::Value *, 8> &CArgs,
        llvm::SmallVector<llvm::Type  *, 8> &CArgTys) {

    ArgsExpr->accept(*CGM);
    llvm::Value *ArgsPtr = ArgsExpr->getCodeGen()->getValue();

    SemaClassType *ArgsType = static_cast<SemaClassType *>(ArgsExpr->getType());
    llvm::StructType *ArgsStructTy = ArgsType->getCodeGen()->getType();

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
            CArg   = Builder->CreateExtractValue(FieldVal, 0);
            CArgTy = llvm::PointerType::getUnqual(CGM->LLVMCtx);
        } else if (FieldTy->isStructTy()) {
            // Class-type fields (e.g. fly.mem.Ptr) in Args structs are assigned via Fly's
            // reference semantics: the field holds a POINTER to the class instance, not
            // the instance value. Dereference through that pointer to get the real i64 addr.
            bool IsClassField = Attr->getType() && Attr->getType()->isClass();
            llvm::Value *Addr;
            if (IsClassField) {
                // Class-type fields hold a Fly reference (pointer to the class instance).
                // Dereference through it to get the real C pointer stored in the instance.
                // BUT: if the field is null (not assigned), pass null to C directly.
                llvm::Value *FieldI64 = Builder->CreateExtractValue(FieldVal, 0);
                llvm::Value *IsNull = Builder->CreateICmpEQ(
                    FieldI64, llvm::ConstantInt::get(CodeGen::Int64Ty, 0));
                // Build the non-null path in a new BB, merge back.
                llvm::Function *CurFn = Builder->GetInsertBlock()->getParent();
                llvm::BasicBlock *OrigBB   = Builder->GetInsertBlock();
                llvm::BasicBlock *NonNullBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "bridge.nonnull", CurFn);
                llvm::BasicBlock *MergeBB  = llvm::BasicBlock::Create(CGM->LLVMCtx, "bridge.merge",   CurFn);
                Builder->CreateCondBr(IsNull, MergeBB, NonNullBB);

                Builder->SetInsertPoint(NonNullBB);
                llvm::Value *InstancePtr = Builder->CreateIntToPtr(
                    FieldI64, llvm::PointerType::getUnqual(CGM->LLVMCtx));
                llvm::Value *InnerVal = Builder->CreateLoad(FieldTy, InstancePtr);
                llvm::Value *InnerAddr = Builder->CreateExtractValue(InnerVal, 0);
                Builder->CreateBr(MergeBB);

                Builder->SetInsertPoint(MergeBB);
                llvm::PHINode *Phi = Builder->CreatePHI(CodeGen::Int64Ty, 2);
                Phi->addIncoming(llvm::ConstantInt::get(CodeGen::Int64Ty, 0), OrigBB); // null → 0
                Phi->addIncoming(InnerAddr, NonNullBB);                                  // non-null → real addr
                Addr = Phi;
            } else {
                Addr = Builder->CreateExtractValue(FieldVal, 0);
            }
            CArg   = Builder->CreateIntToPtr(Addr, llvm::PointerType::getUnqual(CGM->LLVMCtx));
            CArgTy = llvm::PointerType::getUnqual(CGM->LLVMCtx);
        } else {
            CArg   = FieldVal;
            CArgTy = FieldTy;
        }
        CArgs.push_back(CArg);
        CArgTys.push_back(CArgTy);
    }
}

// ─── Emit C call + store result ───────────────────────────────────────────────

void CodeGenStdLibCLang::EmitCCallAndStoreResult(
        const std::string &SymStr,
        llvm::Type *CRetTy,
        bool IsVoid, bool IsPtr, bool IsBool,
        llvm::SmallVector<llvm::Value *, 8> &CArgs,
        llvm::SmallVector<llvm::Type  *, 8> &CArgTys,
        SemaExpr *OutExpr) {

    llvm::FunctionType *FTy = llvm::FunctionType::get(CRetTy, CArgTys, false);
    llvm::FunctionCallee Callee = CGM->Module->getOrInsertFunction(SymStr, FTy);
    llvm::Value *CResult = Builder->CreateCall(Callee, CArgs);

    if (IsVoid) {
        V = CResult;
        return;
    }

    SemaVar *OutVar = static_cast<SemaVar *>(OutExpr);

    if (IsPtr) {
        SemaClassType *PtrType = static_cast<SemaClassType *>(OutVar->getType());
        llvm::StructType *PtrStructTy = PtrType->getCodeGen()->getType();

        // Use malloc so the result struct outlives this method's stack frame.
        // (alloca would create a dangling pointer when the callee returns.)
        llvm::Type *IntPtrTy = CGM->Module->getDataLayout().getIntPtrType(CGM->LLVMCtx);
        llvm::FunctionCallee MallocFn = CGM->Module->getOrInsertFunction(
            "malloc", llvm::FunctionType::get(
                llvm::PointerType::getUnqual(CGM->LLVMCtx), {IntPtrTy}, false));
        llvm::Value *StructSize = llvm::ConstantExpr::getSizeOf(PtrStructTy);
        llvm::Value *StructSizePtr = Builder->CreateIntCast(StructSize, IntPtrTy, false);
        llvm::Value *PtrMem = Builder->CreateCall(MallocFn, {StructSizePtr});

        Builder->CreateStore(PtrMem, OutVar->getCodeGen()->getPointer());
        OutVar->getCodeGen()->resetLoad();

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

// ─── CLang::call() method codegen ─────────────────────────────────────────────

void CodeGenStdLibCLang::GenBridgeMethodCall(SemaCall *Sema) {
    auto &ArgExprs = Sema->getArgs();
    // ArgExprs[0] = sym  (string literal)
    // ArgExprs[1] = const Args args
    // ArgExprs[2] = out  (absent for void variant)

    if (ArgExprs.size() < 2) { V = nullptr; return; }

    if (!isStringLiteral(ArgExprs[0])) { V = nullptr; return; }
    std::string SymStr = static_cast<SemaStringValue *>(ArgExprs[0])->getValue().str();

    SemaNode *ParentNode = Sema->getParent();
    if (!ParentNode) { V = nullptr; return; }
    SemaVar *Instance = static_cast<SemaVar *>(ParentNode);
    if (!Instance->getCodeGen()) { V = nullptr; return; }
    llvm::Value *InstancePtr = Instance->getCodeGen()->getPointer();

    std::string LibStr;
    auto PtrIt = CGM->CLangLibMap.find(InstancePtr);
    if (PtrIt != CGM->CLangLibMap.end()) {
        LibStr = PtrIt->second;
    } else {
        // GEP is recreated on each access — fall back to semantic identity (SemaVar*).
        auto SemaIt = CGM->CLangLibMapBySema.find(Instance);
        if (SemaIt == CGM->CLangLibMapBySema.end()) {
            V = nullptr;
            return;
        }
        LibStr = SemaIt->second;
        // Cache the new pointer so future lookups use the fast path.
        CGM->CLangLibMap[InstancePtr] = LibStr;
    }

    // ─── Determine C return type from the out parameter type (compile-time) ──

    bool IsVoid = (ArgExprs.size() == 2);
    bool IsPtr  = false;
    bool IsBool = false;
    llvm::Type *CRetTy = nullptr;

    if (IsVoid) {
        CRetTy = CodeGen::VoidTy;
    } else {
        SemaType *OutTy = ArgExprs[2]->getType();
        switch (OutTy->getKind()) {
            case SemaKind::TYPE_INTEGER: {
                auto *IT = static_cast<SemaIntType *>(OutTy);
                CRetTy = (IT->getIntKind() == SemaIntTypeKind::TYPE_LONG)
                             ? CodeGen::Int64Ty : CodeGen::Int32Ty;
                break;
            }
            case SemaKind::TYPE_FLOAT: {
                auto *FT = static_cast<SemaFloatType *>(OutTy);
                CRetTy = (FT->getFloatKind() == SemaFloatTypeKind::TYPE_DOUBLE)
                             ? CodeGen::DoubleTy : CodeGen::FloatTy;
                break;
            }
            case SemaKind::TYPE_BOOL:
                CRetTy = CodeGen::Int32Ty; IsBool = true; break;
            case SemaKind::TYPE_CLASS:
                CRetTy = llvm::PointerType::getUnqual(CGM->LLVMCtx);
                IsPtr  = true; break;
            default: V = nullptr; return;
        }
    }

    // ─── Build C argument list from the Args struct ────────────────────────

    llvm::SmallVector<llvm::Value *, 8> CArgs;
    llvm::SmallVector<llvm::Type  *, 8> CArgTys;
    BuildCArgsFromArgsStruct(ArgExprs[1], CArgs, CArgTys);

    // ─── Emit LLVM call + write result into out ────────────────────────────

    EmitCCallAndStoreResult(SymStr, CRetTy, IsVoid, IsPtr, IsBool,
                            CArgs, CArgTys,
                            IsVoid ? nullptr : ArgExprs[2]);
}
