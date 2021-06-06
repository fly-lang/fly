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
#include "CodeGen/CGFunction.h"
#include "CodeGen/CGVar.h"
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
void CodeGenModule::Generate() {
    // Manage Top Decl
    Node.getGlobalVars().begin();
    for (const llvm::StringMapEntry<GlobalVarDecl *> &Entry : Node.getGlobalVars()) {
        GenGlobalVar(Entry.getValue());
    }
    for (const llvm::StringMapEntry<FuncDecl *> &Entry : Node.getFunctions()) {
        GenFunction(Entry.getValue());
    }
}

/**
 * GenStmt from VarDecl
 * @param Decl
 */
CGGlobalVar *CodeGenModule::GenGlobalVar(GlobalVarDecl* VDecl) {
    // Check Value
    llvm::StringRef StrVal;
    if (VDecl->getExpr() && !VDecl->getExpr()->isEmpty()) {
        assert(VDecl->getExpr()->getGroup().size() == 1 &&
                       VDecl->getExpr()->getGroup().at(0)->getKind() == EXPR_VALUE && "Invalid Global Var value");
        ValueExpr *E = static_cast<ValueExpr *>(VDecl->getExpr()->getGroup().at(0));
        StrVal = E->getString();
    }

    CGGlobalVar *CG = new CGGlobalVar(this, VDecl->getType(), StrVal, VDecl->isConstant());
    VDecl->setCodeGen(CG);
    return CG;
}

CGFunction *CodeGenModule::GenFunction(FuncDecl *FDecl) {
    CGFunction *CG = new CGFunction(this, FDecl->getName(), FDecl->getType(), FDecl->getParams(), FDecl->getBody());
    FDecl->setCodeGen(CG);

    // Add Function Body
    if (FDecl->getBody() && !FDecl->getBody()->isEmpty()) {
        for (auto S : FDecl->getBody()->getContent()) {
            GenStmt(S);
        }
    }
    return CG;
}

void CodeGenModule::GenStmt(Stmt * S) {
    switch (S->getKind()) {

        // Var Declaration
        case StmtKind::STMT_VAR_DECL: {
            VarDeclStmt *V = static_cast<VarDeclStmt *>(S);
            CGVar *CG = new CGVar(this, V);
            V->setCodeGen(CG);
            GenExpr(V->getType(), V->getExpr());
            break;
        }

        // Var Assignment
        case STMT_VAR_ASSIGN: {
            VarStmt *V = static_cast<VarStmt *>(S);
            assert(!V->getExpr()->isEmpty() && "Var assign empty");
            GenExpr(V->getVarDecl()->getType(), V->getExpr());
            if (V->getVarDecl()->isGlobal()) {
                GlobalVarDecl *GV = static_cast<GlobalVarDecl *>(V->getVarDecl());
//                GV->getCodeGen();
            } else {
//                V->getVarDecl()->getCodeGen();
            }
//            Builder->CreateStore();
            break;
        }
        case STMT_FUNC_CALL: {
            FuncCallStmt *FCall = static_cast<FuncCallStmt *>(S);
            FCall->getDecl()->getCodeGen()->Call();
             // TODO args
            break;
        }
        case STMT_BLOCK: {
            BlockStmt *BS = static_cast<BlockStmt *>(S);
            switch (BS->getBlockKind()) {
                // TODO
                case BLOCK_STMT:
                    break;
                case BLOCK_STMT_IF:
                    break;
                case BLOCK_STMT_ELSIF:
                    break;
                case BLOCK_STMT_ELSE:
                    break;
                case BLOCK_STMT_SWITCH:
                    break;
                case BLOCK_STMT_CASE:
                    break;
                case BLOCK_STMT_DEFAULT:
                    break;
                case BLOCK_STMT_FOR:
                    break;
            }
            break;
        }
        case DECL_TYPE:
            break;
        case STMT_BREAK:
            break;
        case STMT_CONTINUE:
            break;
        case STMT_RETURN:
            ReturnStmt *R = static_cast<ReturnStmt *>(S);
            if (R->getExpr()->isEmpty())
                Builder->CreateRetVoid();
            else
                GenExpr(R->getContainer()->getType(), R->getExpr());
            break;
    }
}

Type *CodeGenModule::GenTypeValue(const TypeBase *TyData, StringRef StrVal, llvm::Constant *InitVal) {
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

void CodeGenModule::GenExpr(const TypeBase *Typ, GroupExpr *Expr) {
    if (Expr->isEmpty()) {
        return;
    }
    for (auto *E : Expr->getGroup()) {
        switch (E->getKind()) {

            case EXPR_VALUE: {
                ValueExpr *ValEx = static_cast<ValueExpr *>(E);
                Constant *InitVal = nullptr;
                Type *Ty = GenTypeValue(Typ, ValEx->getString(), InitVal);
                break;
            }
            case EXPR_OPERATOR:
                break;
            case EXPR_REF_VAR:
                break;
            case EXPR_REF_FUNC:
                break;
            case EXPR_GROUP:
                GenExpr(Typ, static_cast<GroupExpr *>(E));
                break;
        }
    }
}

CodeGenModule::~CodeGenModule() {
    delete Builder;
}
