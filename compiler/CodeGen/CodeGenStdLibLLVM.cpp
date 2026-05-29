//===--------------------------------------------------------------------------------------------------------------===//
// compiler/CodeGen/CodeGenStdLibLLVM.cpp - fly.llvm intrinsic code generation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenStdLibLLVM.h"

#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"

#include <Sema/SemaCall.h>
#include <Sema/SemaFunctionBase.h>
#include <Sema/SemaParam.h>
#include <Sema/SemaType.h>
#include <Sema/SemaVar.h>

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Intrinsics.h"

using namespace fly;

CodeGenStdLibLLVM::CodeGenStdLibLLVM(CodeGenModule *CGM, llvm::IRBuilder<> *Builder, llvm::Value *&V)
    : CGM(CGM), Builder(Builder), V(V) {}

void CodeGenStdLibLLVM::GenCall(SemaCall *Sema) {
    auto &Params   = Sema->getFunction()->getParams();
    auto &ArgExprs = Sema->getArgs();

    // Input values (const params) fed directly to the intrinsic.
    // Output allocas (non-const params) receive the intrinsic result.
    llvm::SmallVector<llvm::Value *, 8> InArgs;
    llvm::SmallVector<llvm::Value *, 4> OutPtrs;
    llvm::SmallVector<llvm::Type  *, 4> OutTypes;

    for (size_t i = 0; i < ArgExprs.size() && i < Params.size(); i++) {
        Params[i]->getType()->accept(*CGM);
        llvm::Type *ParamTy = Params[i]->getType()->getCodeGen()->getType();

        if (Params[i]->isConstant()) {
            ArgExprs[i]->accept(*CGM);
            llvm::Value *ArgV = ArgExprs[i]->getCodeGen()->getValue();
            if (ArgV->getType()->isPointerTy()) {
                ArgV = Builder->CreateLoad(ParamTy, ArgV);
            } else if (ArgV->getType() != ParamTy) {
                if (ArgV->getType()->isIntegerTy() && ParamTy->isIntegerTy()) {
                    unsigned SrcBits = ArgV->getType()->getIntegerBitWidth();
                    unsigned DstBits = ParamTy->getIntegerBitWidth();
                    bool IsSigned = Params[i]->getType()->isNumber() &&
                        static_cast<SemaIntType *>(Params[i]->getType())->isSigned();
                    ArgV = SrcBits < DstBits
                        ? (IsSigned ? Builder->CreateSExt(ArgV, ParamTy)
                                    : Builder->CreateZExt(ArgV, ParamTy))
                        : Builder->CreateTrunc(ArgV, ParamTy);
                } else if (ArgV->getType()->isFloatingPointTy() && ParamTy->isFloatingPointTy()) {
                    ArgV = Builder->CreateFPCast(ArgV, ParamTy);
                }
            }
            InArgs.push_back(ArgV);
        } else {
            llvm::Value *Ptr = nullptr;
            SemaKind K = ArgExprs[i]->getKind();
            if (K == SemaKind::LOCAL_VAR || K == SemaKind::PARAM_VAR ||
                K == SemaKind::ERROR_VAR || K == SemaKind::ATTRIBUTE ||
                K == SemaKind::INSTANCE_VAR) {
                Ptr = static_cast<SemaVar *>(ArgExprs[i])->getCodeGen()->getPointer();
            } else {
                ArgExprs[i]->accept(*CGM);
                Ptr = ArgExprs[i]->getCodeGen()->getValue();
            }
            OutPtrs.push_back(Ptr);
            OutTypes.push_back(ParamTy);
        }
    }

    // ── Dispatch ──────────────────────────────────────────────────────────────

    llvm::Type *IntrTy = InArgs.empty() ? nullptr : InArgs[0]->getType();
    std::string BaseName(Sema->getFunction()->getName());

    // Float32 variants use a trailing 'f' in Fly but map to the same intrinsic
    if (IntrTy && IntrTy->isFloatTy() && BaseName.size() > 1 && BaseName.back() == 'f')
        BaseName.pop_back();

    // Name mismatches between Fly and LLVM intrinsic names
    if      (BaseName == "fmin") BaseName = "minnum";
    else if (BaseName == "fmax") BaseName = "maxnum";

    // ── String struct primitives (%string = { ptr, i32 }) ─────────────────────

    if (BaseName == "strSize") {
        V = Builder->CreateExtractValue(InArgs[0], 1);
        if (!OutPtrs.empty()) Builder->CreateStore(V, OutPtrs[0]);
        return;
    }
    if (BaseName == "strByteAt") {
        llvm::Value *StrPtr  = Builder->CreateExtractValue(InArgs[0], 0);
        llvm::Value *Idx     = Builder->CreateSExt(InArgs[1], CodeGen::Int64Ty);
        llvm::Value *BytePtr = Builder->CreateGEP(CodeGen::Int8Ty, StrPtr, Idx);
        V = Builder->CreateLoad(CodeGen::Int8Ty, BytePtr);
        if (!OutPtrs.empty()) Builder->CreateStore(V, OutPtrs[0]);
        return;
    }
    if (BaseName == "strGetPtr") {
        llvm::Value *StrPtr = Builder->CreateExtractValue(InArgs[0], 0);
        V = Builder->CreatePtrToInt(StrPtr, CodeGen::Int64Ty);
        if (!OutPtrs.empty()) Builder->CreateStore(V, OutPtrs[0]);
        return;
    }
    if (BaseName == "strMake") {
        llvm::Type  *OpaquePtrTy = llvm::PointerType::getUnqual(CGM->LLVMCtx);
        llvm::Value *Ptr    = Builder->CreateIntToPtr(InArgs[0], OpaquePtrTy);
        llvm::Value *StrVal = llvm::UndefValue::get(CodeGen::StringTy);
        StrVal = Builder->CreateInsertValue(StrVal, Ptr,       0);
        StrVal = Builder->CreateInsertValue(StrVal, InArgs[1], 1);
        if (!OutPtrs.empty()) Builder->CreateStore(StrVal, OutPtrs[0]);
        V = StrVal;
        return;
    }
    if (BaseName == "strPoke") {
        llvm::Type  *OpaquePtrTy = llvm::PointerType::getUnqual(CGM->LLVMCtx);
        llvm::Value *RawPtr  = Builder->CreateIntToPtr(InArgs[0], OpaquePtrTy);
        llvm::Value *Idx     = Builder->CreateSExt(InArgs[1], CodeGen::Int64Ty);
        llvm::Value *BytePtr = Builder->CreateGEP(CodeGen::Int8Ty, RawPtr, Idx);
        V = Builder->CreateStore(InArgs[2], BytePtr);
        return;
    }

    // ── Raw pointer operations ─────────────────────────────────────────────────

    if (BaseName == "ptrByteAt") {
        llvm::Type  *OpaquePtrTy = llvm::PointerType::getUnqual(CGM->LLVMCtx);
        llvm::Value *RawPtr  = Builder->CreateIntToPtr(InArgs[0], OpaquePtrTy);
        llvm::Value *Idx     = Builder->CreateSExt(InArgs[1], CodeGen::Int64Ty);
        llvm::Value *BytePtr = Builder->CreateGEP(CodeGen::Int8Ty, RawPtr, Idx);
        V = Builder->CreateLoad(CodeGen::Int8Ty, BytePtr);
        if (!OutPtrs.empty()) Builder->CreateStore(V, OutPtrs[0]);
        return;
    }
    if (BaseName == "ptrReadByte") {
        llvm::Type  *OpaquePtrTy = llvm::PointerType::getUnqual(CGM->LLVMCtx);
        llvm::Value *RawPtr  = Builder->CreateIntToPtr(InArgs[0], OpaquePtrTy);
        llvm::Value *Idx     = Builder->CreateSExt(InArgs[1], CodeGen::Int64Ty);
        llvm::Value *BytePtr = Builder->CreateGEP(CodeGen::Int8Ty, RawPtr, Idx);
        V = Builder->CreateLoad(CodeGen::Int8Ty, BytePtr);
        if (!OutPtrs.empty()) Builder->CreateStore(V, OutPtrs[0]);
        return;
    }
    if (BaseName == "ptrReadShort") {
        llvm::Type  *OpaquePtrTy = llvm::PointerType::getUnqual(CGM->LLVMCtx);
        llvm::Type  *Int16Ty     = llvm::Type::getInt16Ty(CGM->LLVMCtx);
        llvm::Value *RawPtr  = Builder->CreateIntToPtr(InArgs[0], OpaquePtrTy);
        llvm::Value *Idx     = Builder->CreateSExt(InArgs[1], CodeGen::Int64Ty);
        llvm::Value *BytePtr = Builder->CreateGEP(CodeGen::Int8Ty, RawPtr, Idx);
        V = Builder->CreateLoad(Int16Ty, BytePtr);
        if (!OutPtrs.empty()) Builder->CreateStore(V, OutPtrs[0]);
        return;
    }
    if (BaseName == "ptrReadInt") {
        llvm::Type  *OpaquePtrTy = llvm::PointerType::getUnqual(CGM->LLVMCtx);
        llvm::Value *RawPtr  = Builder->CreateIntToPtr(InArgs[0], OpaquePtrTy);
        llvm::Value *Idx     = Builder->CreateSExt(InArgs[1], CodeGen::Int64Ty);
        llvm::Value *BytePtr = Builder->CreateGEP(CodeGen::Int8Ty, RawPtr, Idx);
        V = Builder->CreateLoad(CodeGen::Int32Ty, BytePtr);
        if (!OutPtrs.empty()) Builder->CreateStore(V, OutPtrs[0]);
        return;
    }
    if (BaseName == "ptrReadLong") {
        llvm::Type  *OpaquePtrTy = llvm::PointerType::getUnqual(CGM->LLVMCtx);
        llvm::Value *RawPtr  = Builder->CreateIntToPtr(InArgs[0], OpaquePtrTy);
        llvm::Value *Idx     = Builder->CreateSExt(InArgs[1], CodeGen::Int64Ty);
        llvm::Value *BytePtr = Builder->CreateGEP(CodeGen::Int8Ty, RawPtr, Idx);
        V = Builder->CreateLoad(CodeGen::Int64Ty, BytePtr);
        if (!OutPtrs.empty()) Builder->CreateStore(V, OutPtrs[0]);
        return;
    }
    if (BaseName == "ptrPokeByte" || BaseName == "ptrPokeShort" ||
        BaseName == "ptrPokeLong" || BaseName == "ptrPokeInt") {
        llvm::Type  *OpaquePtrTy = llvm::PointerType::getUnqual(CGM->LLVMCtx);
        llvm::Value *RawPtr  = Builder->CreateIntToPtr(InArgs[0], OpaquePtrTy);
        llvm::Value *Idx     = Builder->CreateSExt(InArgs[1], CodeGen::Int64Ty);
        llvm::Value *BytePtr = Builder->CreateGEP(CodeGen::Int8Ty, RawPtr, Idx);
        V = Builder->CreateStore(InArgs[2], BytePtr);
        return;
    }
    if (BaseName == "memcmp") {
        llvm::Type  *OpaquePtrTy = llvm::PointerType::getUnqual(CGM->LLVMCtx);
        llvm::Value *A = Builder->CreateIntToPtr(InArgs[0], OpaquePtrTy);
        llvm::Value *B = Builder->CreateIntToPtr(InArgs[1], OpaquePtrTy);
        llvm::FunctionCallee Fn = CGM->Module->getOrInsertFunction(
            "memcmp",
            llvm::FunctionType::get(CodeGen::Int32Ty,
                {OpaquePtrTy, OpaquePtrTy, CodeGen::IntPtrTy}, false));
        V = Builder->CreateCall(Fn, {A, B, InArgs[2]});
        if (!OutPtrs.empty()) Builder->CreateStore(V, OutPtrs[0]);
        return;
    }

    // ── Type conversions ───────────────────────────────────────────────────────

    if (BaseName == "longToInt") {
        V = Builder->CreateTrunc(InArgs[0], CodeGen::Int32Ty);
        if (!OutPtrs.empty()) Builder->CreateStore(V, OutPtrs[0]);
        return;
    }
    if (BaseName == "ulongToLong") {
        V = InArgs[0]; // both i64; reinterpret sign only
        if (!OutPtrs.empty()) Builder->CreateStore(V, OutPtrs[0]);
        return;
    }

    // ── Memory intrinsics (long args are opaque addresses) ────────────────────

    if (BaseName == "memcpy" || BaseName == "memmove" || BaseName == "memset") {
        llvm::Type  *OpaquePtrTy = llvm::PointerType::getUnqual(CGM->LLVMCtx);
        llvm::Value *Dst = Builder->CreateIntToPtr(InArgs[0], OpaquePtrTy);
        if (BaseName == "memcpy") {
            V = Builder->CreateMemCpy(
                Dst, llvm::MaybeAlign(),
                Builder->CreateIntToPtr(InArgs[1], OpaquePtrTy),
                llvm::MaybeAlign(), InArgs[2]);
        } else if (BaseName == "memmove") {
            V = Builder->CreateMemMove(
                Dst, llvm::MaybeAlign(),
                Builder->CreateIntToPtr(InArgs[1], OpaquePtrTy),
                llvm::MaybeAlign(), InArgs[2]);
        } else { // memset: InArgs[1] is i8 byte value
            V = Builder->CreateMemSet(Dst, InArgs[1], InArgs[2], llvm::MaybeAlign());
        }
        return;
    }

    // ── Generic LLVM intrinsic fallback ───────────────────────────────────────

    llvm::Intrinsic::ID IntrID = llvm::Intrinsic::lookupIntrinsicID("llvm." + BaseName);
    llvm::SmallVector<llvm::Type *, 2> TyArgs;
    if (IntrTy) TyArgs.push_back(IntrTy);
    llvm::Function *IntrFn = llvm::Intrinsic::getOrInsertDeclaration(
        CGM->Module, IntrID, TyArgs);

    // ctlz/cttz require an extra i1 is_zero_undef argument (always false)
    if (IntrID == llvm::Intrinsic::ctlz || IntrID == llvm::Intrinsic::cttz)
        InArgs.push_back(llvm::ConstantInt::getFalse(CGM->LLVMCtx));

    llvm::Value *Result = Builder->CreateCall(IntrFn, InArgs);

    // Store result into the output param alloca, coercing integer width if needed
    if (!OutPtrs.empty()) {
        llvm::Value *StoreVal = Result;
        if (StoreVal->getType() != OutTypes[0] &&
            StoreVal->getType()->isIntegerTy() && OutTypes[0]->isIntegerTy()) {
            unsigned SrcBits = StoreVal->getType()->getIntegerBitWidth();
            unsigned DstBits = OutTypes[0]->getIntegerBitWidth();
            StoreVal = SrcBits > DstBits
                ? Builder->CreateTrunc(StoreVal, OutTypes[0])
                : Builder->CreateZExt(StoreVal, OutTypes[0]);
        }
        Builder->CreateStore(StoreVal, OutPtrs[0]);
    }
    V = Result;
}
