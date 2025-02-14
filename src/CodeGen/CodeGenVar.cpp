//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CGVar.cpp - Code Generator Block implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "CodeGen/CodeGenVar.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenClass.h"
#include "CodeGen/CodeGenModule.h"

using namespace fly;

//CodeGenVar::CodeGenVar(CodeGenModule *CGM, ASTVar *Var) : CGM(CGM) {
//    this->T = Var->getType()->getKind() == ASTTypeKind::TYPE_BOOL ? CGM->Int8Ty : CGM->GenType(Var->getType());
//}

CodeGenVar::CodeGenVar(CodeGenModule *CGM, llvm::Type *T) : CGM(CGM), T(T) {

}

CodeGenVar::CodeGenVar(CodeGenModule *CGM, llvm::Type *Ty, CodeGenVar *Parent, uint32_t Index) : CodeGenVar(CGM, Ty) {
    this->Parent = Parent;
    this->Index = Index;
}

CodeGenVar *CodeGenVar::getParent() {
    return Parent;
}

CodeGenVarBase *CodeGenVar::getVar(llvm::StringRef Name) {
    return Vars.lookup(Name);
}

llvm::Type *CodeGenVar::getType() {
    return T;
}

llvm::AllocaInst *CodeGenVar::Alloca() {
    if (this->T->isStructTy()) { // FIXME ?? with this->T->isStructTy()
        llvm::PointerType *PtrTy = T->getPointerTo(CGM->Module->getDataLayout().getAllocaAddrSpace());
        this->Pointer = CGM->Builder->CreateAlloca(PtrTy);
    } else {
        this->Pointer = CGM->Builder->CreateAlloca(T->isIntegerTy(1) ? CGM->Int8Ty : T);
    }
    return this->Pointer;
}

llvm::StoreInst *CodeGenVar::Store(llvm::Value *Val) {
    this->BlockID = CGM->Builder->GetInsertBlock()->getName();
    this->LoadI = nullptr;

    // Fix Architecture Compatibility of bool i1 to i8
    if (T->isIntegerTy(1)) {
        Val = CGM->Builder->CreateZExt(Val, CGM->Int8Ty);
    }
    if (Pointer == nullptr)
        return CGM->Builder->CreateStore(Val, getPointer());
    else
        return CGM->Builder->CreateStore(Val, Pointer);
}

llvm::LoadInst *CodeGenVar::Load() {
    this->BlockID = CGM->Builder->GetInsertBlock()->getName();
    this->LoadI = CGM->Builder->CreateLoad(getPointer());
    return this->LoadI;
}

llvm::Value *CodeGenVar::getValue() {
    if (!this->LoadI || this->BlockID != CGM->Builder->GetInsertBlock()->getName()) {
        return Load();
    }
    return this->LoadI;
}

llvm::Value *CodeGenVar::getPointer() {
    if (Parent) {
        ConstantInt *Zero = llvm::ConstantInt::get(CGM->Int32Ty, 0);
        ConstantInt *Idx = llvm::ConstantInt::get(CGM->Int32Ty, Index);
        return CGM->Builder->CreateInBoundsGEP(Parent->getType(), Parent->getValue(), {Zero, Idx});
    }
    return this->Pointer;
}

void CodeGenVar::addVar(StringRef Name, CodeGenVarBase *CGV) {
    Vars.insert(std::make_pair(Name, CGV));
}
