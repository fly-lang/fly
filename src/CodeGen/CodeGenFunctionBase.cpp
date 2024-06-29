//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGFunction.cpp - Code Generator Function implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenFunctionBase.h"
#include "CodeGen/CodeGenEnumEntry.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGenError.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTFunction.h"
#include "AST/ASTParam.h"
#include "AST/ASTCall.h"
#include "AST/ASTType.h"
#include "AST/ASTIdentityType.h"
#include "AST/ASTClass.h"
#include "AST/ASTClassMethod.h"
#include "AST/ASTClassAttribute.h"
#include "Basic/Debug.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/ADT/StringRef.h"

using namespace fly;

CodeGenFunctionBase::CodeGenFunctionBase(CodeGenModule *CGM, ASTFunctionBase *AST) : CGM(CGM), AST(AST) {

}

CodeGenModule *CodeGenFunctionBase::getCodeGenModule() {
    return CGM;
}

void CodeGenFunctionBase::GenReturnType() {
    RetType = CGM->GenType(AST->getReturnType());
}

void CodeGenFunctionBase::GenParamTypes(CodeGenModule * CGM, llvm::SmallVector<llvm::Type *, 8> &Types, llvm::SmallVector<ASTParam *, 8> Params) {
    // Populate Types by reference
    if (Params.empty()) {
        return;
    }
    if (!Params.empty()) {
        for (auto Param : Params) {
            llvm::Type *ParamTy = CGM->GenType(Param->getType());
            Types.push_back(ParamTy);
        }
    }
//    if (Params->getEllipsis() != nullptr) {
        // TODO
//    }
}

ASTFunctionBase *CodeGenFunctionBase::getAST() {
    return AST;
}

llvm::StringRef CodeGenFunctionBase::getName() const {
    return Fn->getName();
}

llvm::Function *CodeGenFunctionBase::getFunction() {
    return Fn;
}

llvm::FunctionType *CodeGenFunctionBase::getFunctionType() {
    return FnType;
}

void CodeGenFunctionBase::setInsertPoint() {
    FLY_DEBUG("CodeGenFunctionBase", "setInsertPoint");
    Entry = BasicBlock::Create(CGM->LLVMCtx, "entry", Fn);
    CGM->Builder->SetInsertPoint(Entry);
}

void CodeGenFunctionBase::AllocaErrorHandler() {
    // Alloca ErrorHandler
    CodeGenError *CGE = CGM->GenErrorHandler(AST->getErrorHandler());
    AST->getErrorHandler()->setCodeGen(CGE);
}

void CodeGenFunctionBase::AllocaLocalVars() {

    // Allocation of declared ASTParam
    for (auto &Param: AST->getParams()) {
        CodeGenVar *CGV = CGM->GenLocalVar(Param);
        CGV->Alloca();
        Param->setCodeGen(CGV);
    }

    // Allocation of all declared ASTLocalVar
    for (auto &LocalVar: AST->getLocalVars()) {
        CodeGenVar *CGV = CGM->GenLocalVar(LocalVar);
        CGV->Alloca();

        // Create CodeGenVar for all inner attributes of a class
        if (LocalVar->getType()->isIdentity()) {
            ASTIdentityType *IdentityType = (ASTIdentityType *) LocalVar->getType();
            if (IdentityType->isClass()) {
                ASTClass * Class =  (ASTClass *) IdentityType->getDef();
                uint32_t Idx = 1; // 0 is the vtable type
                for (auto &AttrEntry : Class->getAttributes()) {
                    ASTClassAttribute *Attr = AttrEntry.getValue();
                    CodeGenVar *CGAttr = new CodeGenVar(CGM, CGM->GenType(Attr->getType()), CGV, Idx);
                    CGV->addVar(Attr->getName(), CGAttr);
                    Attr->setCodeGen(CGAttr);
                    Idx++;
                }
            }
            
        }
        LocalVar->setCodeGen(CGV);
    }
}

void CodeGenFunctionBase::StoreErrorHandler(bool isMain) {
    if (!isMain)
        ((CodeGenError *) AST->getErrorHandler()->getCodeGen())->StorePointer(Fn->getArg(0));
}

void CodeGenFunctionBase::StoreParams(bool isMain) {
    // Store Param Values (n = 0 is the Error param)
    int n = isMain ? 0 : 1;

    for (auto &Param: AST->getParams()) {

        // Store Arg value into Param
        CodeGenVarBase *CGV = Param->getCodeGen();
        CGV->Store(Fn->getArg(n));

        // TODO if Arg is not present store the Param default value
//        if (Param->getExpr()) {
//            CGM->GenExpr(this, Param->getType(), Param->getExpr());
//        }
        ++n;
    }
}
