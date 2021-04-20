//===-------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenModule.cpp - Emit LLVM Code from ASTs for a Module
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
/// \file
/// Defines the fly::CodeGenModule builder.
/// This builds an AST and converts it to LLVM Code.
///
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CharUnits.h"

using namespace llvm;
using namespace fly;

/// toCharUnitsFromBits - Convert a size in bits to a size in characters.
CharUnits toCharUnitsFromBits(int64_t BitSize) {
    return CharUnits::fromQuantity(BitSize / 8);
}

CodeGenModule::CodeGenModule(DiagnosticsEngine &Diags, ASTNode &AST, TargetInfo &Target) :
                                    Diags(Diags),
                                    AST(AST),
                                    Target(Target),
                                    VMContext(),
                                    Module(new llvm::Module(AST.getFileName(), VMContext)) {
    // Configure Types
    VoidTy = llvm::Type::getVoidTy(VMContext);
    BoolTy = llvm::Type::getInt1Ty(VMContext);
    Int8Ty = llvm::Type::getInt8Ty(VMContext);
    Int16Ty = llvm::Type::getInt16Ty(VMContext);
    Int32Ty = llvm::Type::getInt32Ty(VMContext);
    Int64Ty = llvm::Type::getInt64Ty(VMContext);
    HalfTy = llvm::Type::getHalfTy(VMContext);
    BFloatTy = llvm::Type::getBFloatTy(VMContext);
    FloatTy = llvm::Type::getFloatTy(VMContext);
    DoubleTy = llvm::Type::getDoubleTy(VMContext);
    PointerWidthInBits = Target.getPointerWidth(0);
    PointerAlignInBytes = toCharUnitsFromBits(Target.getPointerAlign(0)).getQuantity();
    SizeSizeInBytes = toCharUnitsFromBits(Target.getMaxPointerWidth()).getQuantity();
    IntAlignInBytes = toCharUnitsFromBits(Target.getIntAlign()).getQuantity();
    IntTy = llvm::IntegerType::get(VMContext, Target.getIntWidth());
    IntPtrTy = llvm::IntegerType::get(VMContext, Target.getMaxPointerWidth());
    Int8PtrTy = Int8Ty->getPointerTo(0);
    Int8PtrPtrTy = Int8PtrTy->getPointerTo(0);
    AllocaInt8PtrTy = Int8Ty->getPointerTo(Module->getDataLayout().getAllocaAddrSpace());

    // Configure Module
    Module->setTargetTriple(Target.getTriple().getTriple());
    Module->setDataLayout(Target.getDataLayout());
    const auto &SDKVersion = Target.getSDKVersion();
    if (!SDKVersion.empty())
        Module->setSDKVersion(SDKVersion);

    // todo Add dependencies, Linker Options
}

/**
 * Generate from ASTContext
 */
void CodeGenModule::Generate() {
    // Manage Package

    // Manage Imports

    // Generate Vars

    // Generate Functions
}

/**
 * Generate from VarDecl
 * @param Decl
 */
void CodeGenModule::GenerateGlobalVar(VarDecl* Var) {
    llvm::Type *T = nullptr;
    const TypeDecl *ty = Var->getType();
    switch (ty->getKind()) {

        case Int:
            T = Int32Ty;
            break;
        case Float:
            T = FloatTy;
            break;
        case Boolean:
            T = BoolTy;
            break;
    }

    assert(T != nullptr && "Missing Var Type");
    Module->getOrInsertGlobal(Var->getName(), T);

}

GlobalVariable *CodeGenModule::GenerateAndGetGlobalVar(GlobalVarDecl* Var) {
    GenerateGlobalVar(Var);
    return Module->getGlobalVariable(Var->getName());
}

/**
 * Generate from SmallVector of VarDecl
 * @param Vars
 */
void CodeGenModule::GenerateGlobalVars(std::vector<GlobalVarDecl*> Vars) {
    for (GlobalVarDecl *Var : Vars) {
        GenerateGlobalVar(Var);
    }
}
