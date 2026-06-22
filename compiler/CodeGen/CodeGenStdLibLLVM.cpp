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
#include <Sema/SemaClassType.h>
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

    // A class element that is not a value STRUCT is a reference type: it lives on the heap
    // and is represented everywhere as an 8-byte handle (pointer), e.g. the `ptr` field of
    // Wrapper<SomeClass>. The typed-slot intrinsics must therefore store/load the handle,
    // not copy the pointee struct by value (which would slice subclasses and break identity).
    auto isRefClass = [](SemaType *T) {
        return T->isClass() &&
               static_cast<SemaClassType *>(T)->getClassKind() != SemaClassKind::STRUCT;
    };
    llvm::Type *OpaquePtrTy = llvm::PointerType::getUnqual(CGM->LLVMCtx);

    for (size_t i = 0; i < ArgExprs.size() && i < Params.size(); i++) {
        Params[i]->getType()->accept(*CGM);
        bool IsRef = isRefClass(Params[i]->getType());
        llvm::Type *ParamTy = IsRef ? OpaquePtrTy
                                    : Params[i]->getType()->getCodeGen()->getType();

        if (Params[i]->isConstant()) {
            ArgExprs[i]->accept(*CGM);
            llvm::Value *ArgV = ArgExprs[i]->getCodeGen()->getValue();
            if (IsRef) {
                // Reference handle: getValue() already yields the heap pointer; pass it as-is
                // (an 8-byte handle) so slotSizeT→8 and slotPokeT/ReadT store/load the pointer.
            } else if (ArgV->getType()->isPointerTy()) {
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

    // ── Generic typed-slot intrinsics ─────────────────────────────────────────
    // These are declared generic (slotPokeT<T> etc.) in llvm.fly.h. By the time we
    // reach CodeGen the call has been monomorphized to a concrete specialization, so
    // the element type T is carried by the coerced argument value (InArgs) and the
    // output param type (OutTypes) — both already concrete (int, long, double,
    // string struct, pointer, …). One source intrinsic thus serves every T.
    if (BaseName.rfind("slotHashT", 0) == 0 || BaseName.rfind("slotHashAtT", 0) == 0) {
        // slotHashT(const T key, int out)                          → hash a value
        // slotHashAtT(const long buf, const int off, T sample, out) → hash a stored slot
        //  string → FNV-1a over bytes; integer → value; pointer → address.
        // Equality is handled by Fly '==' / slotCmpT.
        auto hashValue = [&](llvm::Value *Key) -> llvm::Value * {
            llvm::Type *KeyTy = Key->getType();
            if (KeyTy == CodeGen::StringTy) {
                llvm::Value *Ptr  = Builder->CreateExtractValue(Key, 0);
                llvm::Value *Size = Builder->CreateExtractValue(Key, 1); // i32
                llvm::Function *Fn = Builder->GetInsertBlock()->getParent();
                llvm::BasicBlock *PreBB  = Builder->GetInsertBlock();
                llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "hash.loop", Fn);
                llvm::BasicBlock *BodyBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "hash.body", Fn);
                llvm::BasicBlock *ExitBB = llvm::BasicBlock::Create(CGM->LLVMCtx, "hash.exit", Fn);
                llvm::Value *Init = llvm::ConstantInt::get(CodeGen::Int32Ty, 2166136261u);
                llvm::Value *Prime = llvm::ConstantInt::get(CodeGen::Int32Ty, 16777619u);
                Builder->CreateBr(LoopBB);
                Builder->SetInsertPoint(LoopBB);
                llvm::PHINode *IPhi = Builder->CreatePHI(CodeGen::Int32Ty, 2);
                llvm::PHINode *HPhi = Builder->CreatePHI(CodeGen::Int32Ty, 2);
                IPhi->addIncoming(llvm::ConstantInt::get(CodeGen::Int32Ty, 0), PreBB);
                HPhi->addIncoming(Init, PreBB);
                Builder->CreateCondBr(Builder->CreateICmpSLT(IPhi, Size), BodyBB, ExitBB);
                Builder->SetInsertPoint(BodyBB);
                llvm::Value *BPtr = Builder->CreateGEP(CodeGen::Int8Ty, Ptr,
                    Builder->CreateSExt(IPhi, CodeGen::Int64Ty));
                llvm::Value *B = Builder->CreateZExt(Builder->CreateLoad(CodeGen::Int8Ty, BPtr), CodeGen::Int32Ty);
                llvm::Value *H = Builder->CreateMul(Builder->CreateXor(HPhi, B), Prime);
                IPhi->addIncoming(Builder->CreateAdd(IPhi, llvm::ConstantInt::get(CodeGen::Int32Ty, 1)), BodyBB);
                HPhi->addIncoming(H, BodyBB);
                Builder->CreateBr(LoopBB);
                Builder->SetInsertPoint(ExitBB);
                return HPhi;
            }
            if (KeyTy->isPointerTy())
                return Builder->CreateTrunc(Builder->CreatePtrToInt(Key, CodeGen::Int64Ty), CodeGen::Int32Ty);
            if (KeyTy->isIntegerTy()) {
                unsigned Bits = KeyTy->getIntegerBitWidth();
                return Bits == 32 ? Key
                     : (Bits < 32 ? Builder->CreateZExt(Key, CodeGen::Int32Ty)
                                  : Builder->CreateTrunc(Key, CodeGen::Int32Ty));
            }
            return llvm::ConstantInt::get(CodeGen::Int32Ty, 0);
        };
        llvm::Value *Key;
        if (BaseName.rfind("slotHashAtT", 0) == 0) {
            llvm::Type  *OpaquePtrTy = llvm::PointerType::getUnqual(CGM->LLVMCtx);
            llvm::Value *RawPtr  = Builder->CreateIntToPtr(InArgs[0], OpaquePtrTy);
            llvm::Value *Idx     = Builder->CreateSExt(InArgs[1], CodeGen::Int64Ty);
            llvm::Value *BytePtr = Builder->CreateGEP(CodeGen::Int8Ty, RawPtr, Idx);
            Key = Builder->CreateLoad(InArgs[2]->getType(), BytePtr);
        } else {
            Key = InArgs[0];
        }
        V = hashValue(Key);
        if (!OutPtrs.empty()) Builder->CreateStore(V, OutPtrs[0]);
        return;
    }
    if (BaseName.rfind("slotCmpT", 0) == 0) {
        // slotCmpT(const long buf, const int byteOff, const T key, int out): 3-way compare
        // of the stored element vs key. string → lexicographic (memcmp + length tiebreak);
        // integer → signed; pointer → address. No clone (safe for hash/tree probing).
        llvm::Type  *OpaquePtrTy = llvm::PointerType::getUnqual(CGM->LLVMCtx);
        llvm::Value *RawPtr  = Builder->CreateIntToPtr(InArgs[0], OpaquePtrTy);
        llvm::Value *Idx     = Builder->CreateSExt(InArgs[1], CodeGen::Int64Ty);
        llvm::Value *BytePtr = Builder->CreateGEP(CodeGen::Int8Ty, RawPtr, Idx);
        llvm::Value *Key     = InArgs[2];
        llvm::Type  *ElemTy  = Key->getType();
        llvm::Value *Stored  = Builder->CreateLoad(ElemTy, BytePtr);
        llvm::Value *Zero    = llvm::ConstantInt::get(CodeGen::Int32Ty, 0);
        llvm::Value *One     = llvm::ConstantInt::get(CodeGen::Int32Ty, 1);
        llvm::Value *NegOne  = llvm::ConstantInt::get(CodeGen::Int32Ty, -1);
        if (ElemTy == CodeGen::StringTy) {
            llvm::Value *SP = Builder->CreateExtractValue(Stored, 0);
            llvm::Value *SS = Builder->CreateExtractValue(Stored, 1); // i32
            llvm::Value *KP = Builder->CreateExtractValue(Key, 0);
            llvm::Value *KS = Builder->CreateExtractValue(Key, 1);
            llvm::Value *MinS = Builder->CreateSelect(Builder->CreateICmpSLT(SS, KS), SS, KS);
            llvm::FunctionCallee MemcmpFn = CGM->Module->getOrInsertFunction(
                "memcmp", llvm::FunctionType::get(CodeGen::Int32Ty,
                    {OpaquePtrTy, OpaquePtrTy, CodeGen::IntPtrTy}, false));
            llvm::Value *C = Builder->CreateCall(MemcmpFn,
                {SP, KP, Builder->CreateZExt(MinS, CodeGen::IntPtrTy)});
            // tie on shared prefix → compare lengths
            llvm::Value *LenDiff = Builder->CreateSub(SS, KS);
            V = Builder->CreateSelect(Builder->CreateICmpNE(C, Zero), C, LenDiff);
        } else if (ElemTy->isPointerTy()) {
            llvm::Value *A = Builder->CreatePtrToInt(Stored, CodeGen::Int64Ty);
            llvm::Value *B = Builder->CreatePtrToInt(Key, CodeGen::Int64Ty);
            V = Builder->CreateSelect(Builder->CreateICmpULT(A, B), NegOne,
                Builder->CreateSelect(Builder->CreateICmpUGT(A, B), One, Zero));
        } else {
            V = Builder->CreateSelect(Builder->CreateICmpSLT(Stored, Key), NegOne,
                Builder->CreateSelect(Builder->CreateICmpSGT(Stored, Key), One, Zero));
        }
        if (!OutPtrs.empty()) Builder->CreateStore(V, OutPtrs[0]);
        return;
    }
    if (BaseName.rfind("slotSizeT", 0) == 0) {
        // slotSizeT(const T sample, int out): out = sizeof(T) in bytes
        llvm::Type *ElemTy = InArgs[0]->getType();
        uint64_t Sz = CGM->Module->getDataLayout().getTypeAllocSize(ElemTy).getFixedValue();
        V = llvm::ConstantInt::get(CodeGen::Int32Ty, Sz);
        if (!OutPtrs.empty()) Builder->CreateStore(V, OutPtrs[0]);
        return;
    }
    if (BaseName.rfind("slotPokeT", 0) == 0 || BaseName.rfind("slotReadT", 0) == 0 ||
        BaseName.rfind("slotFreeT", 0) == 0) {
        llvm::Type  *OpaquePtrTy = llvm::PointerType::getUnqual(CGM->LLVMCtx);
        llvm::Value *RawPtr  = Builder->CreateIntToPtr(InArgs[0], OpaquePtrTy);
        llvm::Value *Idx     = Builder->CreateSExt(InArgs[1], CodeGen::Int64Ty);
        llvm::Value *BytePtr = Builder->CreateGEP(CodeGen::Int8Ty, RawPtr, Idx);

        // Deep-clone a %string value ({ptr,i32}) into a fresh malloc'd buffer, so each
        // owner frees an independent buffer (no aliasing → no double free). Mirrors
        // CodeGenExpr::GenStringClone. No-op for any non-string element type.
        auto cloneIfString = [&](llvm::Value *Val) -> llvm::Value * {
            if (Val->getType() != CodeGen::StringTy) return Val;
            llvm::Value *SrcPtr  = Builder->CreateExtractValue(Val, 0);
            llvm::Value *Size    = Builder->CreateExtractValue(Val, 1);
            llvm::Value *SizeExt = Builder->CreateZExt(Size, CodeGen::IntPtrTy);
            llvm::FunctionCallee MallocFn = CGM->Module->getOrInsertFunction(
                "malloc", llvm::FunctionType::get(OpaquePtrTy, {CodeGen::IntPtrTy}, false));
            llvm::Value *HeapPtr = Builder->CreateCall(MallocFn, {SizeExt});
            Builder->CreateMemCpy(HeapPtr, llvm::MaybeAlign(), SrcPtr, llvm::MaybeAlign(), SizeExt);
            llvm::Value *R = llvm::UndefValue::get(CodeGen::StringTy);
            R = Builder->CreateInsertValue(R, HeapPtr, 0);
            R = Builder->CreateInsertValue(R, Size, 1);
            return R;
        };

        if (BaseName.rfind("slotPokeT", 0) == 0) {
            // slotPokeT(const long buf, const int byteOff, const T val): clone owned values
            V = Builder->CreateStore(cloneIfString(InArgs[2]), BytePtr);
            return;
        }
        if (BaseName.rfind("slotReadT", 0) == 0) {
            // slotReadT(const long buf, const int byteOff, T out): return a fresh clone
            llvm::Type *ElemTy = OutTypes.empty() ? CodeGen::Int64Ty : OutTypes[0];
            V = cloneIfString(Builder->CreateLoad(ElemTy, BytePtr));
            if (!OutPtrs.empty()) Builder->CreateStore(V, OutPtrs[0]);
            return;
        }
        // slotFreeT(const long buf, const int byteOff, const T sample): free owned element.
        // Only owned-value types (string) are released; trivial and handle types are no-ops
        // (handles/refs are borrowed — the container never frees the pointee).
        llvm::Type *ElemTy = InArgs.size() > 2 ? InArgs[2]->getType() : CodeGen::Int64Ty;
        if (ElemTy == CodeGen::StringTy) {
            llvm::Value *StrVal = Builder->CreateLoad(CodeGen::StringTy, BytePtr);
            llvm::Value *StrPtr = Builder->CreateExtractValue(StrVal, 0);
            llvm::FunctionCallee FreeFn = CGM->Module->getOrInsertFunction(
                "free", llvm::FunctionType::get(llvm::Type::getVoidTy(CGM->LLVMCtx), {OpaquePtrTy}, false));
            V = Builder->CreateCall(FreeFn, {StrPtr});
        }
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
