//===--------------------------------------------------------------------------------------------------------------===//
// compiler/CodeGen/CodeGenStdLibRuntime.cpp - fly.runtime C symbol code generation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenStdLibRuntime.h"

#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"

#include <Sema/SemaCall.h>
#include <Sema/SemaFunctionBase.h>
#include <Sema/SemaParam.h>
#include <Sema/SemaType.h>
#include <Sema/SemaVar.h>

#include "llvm/ADT/SmallVector.h"

using namespace fly;

CodeGenStdLibRuntime::CodeGenStdLibRuntime(CodeGenModule *CGM, llvm::IRBuilder<> *Builder, llvm::Value *&V)
    : CGM(CGM), Builder(Builder), V(V) {}

void CodeGenStdLibRuntime::GenCall(SemaCall *Sema) {
    auto &Params   = Sema->getFunction()->getParams();
    auto &ArgExprs = Sema->getArgs();

    // Const params become C arguments; non-const params provide the output alloca.
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

    // ── Emit C call ───────────────────────────────────────────────────────────
    // Call C function by exact Fly name (no mangling, no error context).
    // Return type is inferred from the output param type; void if none.

    llvm::Type *CRetTy = OutTypes.empty() ? CodeGen::VoidTy : OutTypes[0];
    llvm::SmallVector<llvm::Type *, 8> CParamTypes;
    for (auto *A : InArgs) CParamTypes.push_back(A->getType());

    llvm::FunctionType *FnTy = llvm::FunctionType::get(CRetTy, CParamTypes, false);
    std::string CName(Sema->getFunction()->getName());
    llvm::FunctionCallee Callee = CGM->Module->getOrInsertFunction(CName, FnTy);
    llvm::Value *CResult = Builder->CreateCall(Callee, InArgs);

    // Store return value into the output param alloca with type coercion
    if (!OutPtrs.empty() && !CRetTy->isVoidTy()) {
        llvm::Value *StoreVal = CResult;
        llvm::Type  *OutTy   = OutTypes[0];
        if (StoreVal->getType() != OutTy) {
            if (StoreVal->getType()->isPointerTy() && OutTy->isIntegerTy()) {
                StoreVal = Builder->CreatePtrToInt(StoreVal, OutTy);
            } else if (StoreVal->getType()->isIntegerTy() && OutTy->isIntegerTy()) {
                unsigned SrcBits = StoreVal->getType()->getIntegerBitWidth();
                unsigned DstBits = OutTy->getIntegerBitWidth();
                StoreVal = SrcBits > DstBits
                    ? Builder->CreateTrunc(StoreVal, OutTy)
                    : Builder->CreateSExt(StoreVal, OutTy);
            }
        }
        Builder->CreateStore(StoreVal, OutPtrs[0]);
    }
    V = CResult;
}
