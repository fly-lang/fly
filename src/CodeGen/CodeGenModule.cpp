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
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Value.h"
#include<iostream>

using namespace llvm;
using namespace fly;

/// toCharUnitsFromBits - Convert a size in bits to a size in characters.
CharUnits toCharUnitsFromBits(int64_t BitSize) {
    return CharUnits::fromQuantity(BitSize / 8);
}

CodeGenModule::CodeGenModule(DiagnosticsEngine &Diags, ASTNode &Node, TargetInfo &Target) :
        Diags(Diags),
        Node(Node),
        Target(Target),
        VMContext(),
        Builder(new IRBuilder<>(VMContext)),
        Module(new llvm::Module(Node.getFileName(), VMContext)) {
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
 * GenStmt from ASTContext
 */
void CodeGenModule::GenAST() {
    // Manage Top Decl
    Node.getGlobalVars().begin();
    for (const llvm::StringMapEntry<GlobalVarDecl *> &Entry : Node.getGlobalVars()) {
        GenTop(Entry.getValue());
    }
    for (const llvm::StringMapEntry<FuncDecl *> &Entry : Node.getFunctions()) {
        GenTop(Entry.getValue());
    }
}

/**
 * GenStmt from VarDecl
 * @param Decl
 */
llvm::GlobalVariable *CodeGenModule::GenTop(GlobalVarDecl* V) {
    // Check Value
    StringRef StrVal;

    if (V->getExpr() && !V->getExpr()->isEmpty()) {
        if (V->getExpr()->getGroup().at(0)->getKind() == EXPR_VALUE) {
            StrVal = static_cast<ValueExpr *>(V->getExpr()->getGroup().at(0))->getString();
        }
    }

    llvm::Constant *InitVal = nullptr;
    Type *Ty = GenType(V->getType(), StrVal, InitVal);
    GlobalVariable *G = new llvm::GlobalVariable(*Module, Ty, V->isConstant(), GlobalValue::ExternalLinkage,
                                                 InitVal,
                                                 V->getName());
    return G;
}

Function *CodeGenModule::GenTop(FuncDecl *Decl) {
    FunctionType *FnTy;
    const TypeBase *RetTyData = Decl->getType();
    if (Decl->getParams()->getVars().empty()) {
        FnTy = FunctionType::get(GenType(RetTyData), Decl->getParams()->getVarArg() != nullptr);
    } else {
        SmallVector<Type *, 8> ArrayParams;
        for (auto Param : Decl->getParams()->getVars()) {
            Type *ParamTy = GenType(Param->getType());
            ArrayParams.push_back(ParamTy);
        }
        FnTy = FunctionType::get(GenType(RetTyData), ArrayParams,
                                 Decl->getParams()->getVarArg() != nullptr);
    }

    Function *Fn = llvm::Function::Create(FnTy, llvm::Function::ExternalLinkage, Decl->getName());
    if (Decl->getBody() && !Decl->getBody()->isEmpty()) {
        BasicBlock *BB = BasicBlock::Create(VMContext, "entry", Fn);
        for (auto S : Decl->getBody()->getContent()) {
            Builder->SetInsertPoint(BB);
            GenStmt(S);
        }
    }
    return Fn;
}

void CodeGenModule::GenStmt(Stmt * S) {
    switch (S->getKind()) {
        case StmtKind::STMT_VAR_DECL:
            break;
        case STMT_VAR_ASSIGN:
            break;
        case STMT_FUNC_CALL:
            break;
        case STMT_BLOCK:
            break;
        case DECL_TYPE:
            break;
        case STMT_BREAK:
            break;
        case STMT_CONTINUE:
            break;
        case STMT_RETURN:
            break;
    }
}

Type *CodeGenModule::GenType(const TypeBase *TyData, StringRef StrVal, llvm::Constant *InitVal) {
    // Check Type
    llvm::Type *Ty = nullptr;
    switch (TyData->getKind()) {

        case TYPE_VOID:
            Ty = VoidTy;
            break;
        case TYPE_INT:
            Ty = Int32Ty;
            if (!StrVal.empty()) {
                uint64_t intVal = std::stoi(StrVal.str());
                InitVal = llvm::ConstantInt::get(Ty, intVal, true);
            }
            break;
        case TYPE_FLOAT:
            Ty = FloatTy;
            if (!StrVal.empty()) {
                InitVal = llvm::ConstantFP::get(Ty, StrVal);
            }
            break;
        case TYPE_BOOL:
            Ty = BoolTy;
            if (!StrVal.empty()) {
                if (StrVal.equals("true")) {
                    InitVal = llvm::ConstantInt::get(Ty, 1, false);
                } else if (StrVal.equals("false")) {
                    InitVal = llvm::ConstantInt::get(Ty, 0, false);
                } else {
                    // TODO error bad Bool value
                }
            }
            break;
    }

    assert(Ty != nullptr && "Missing Var Type");
    return Ty;
}

CodeGenModule::~CodeGenModule() {
    delete Builder;
}

