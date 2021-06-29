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
#include "CodeGen/CodeGenFunction.h"
#include "CodeGen/CodeGenCall.h"
#include "CodeGen/CodeGenGlobalVar.h"
#include "CodeGen/CodeGenVar.h"
#include "AST/ASTNode.h"
#include "AST/VarDeclStmt.h"
#include "AST/GlobalVarDecl.h"
#include "AST/BlockStmt.h"
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

CodeGenModule::CodeGenModule(DiagnosticsEngine &Diags, ASTNode &Node, LLVMContext &LLVMCtx, TargetInfo &Target, CodeGenOptions &CGOpts) :
        Diags(Diags),
        Node(Node),
        Target(Target),
        Module(new llvm::Module(Node.getFileName(), LLVMCtx)),
        LLVMCtx(LLVMCtx),
        Builder(new IRBuilder<>(LLVMCtx)),
        CGOpts(CGOpts) {
    // Configure Types
    VoidTy = llvm::Type::getVoidTy(LLVMCtx);
    BoolTy = llvm::Type::getInt1Ty(LLVMCtx);
    Int8Ty = llvm::Type::getInt8Ty(LLVMCtx);
    Int16Ty = llvm::Type::getInt16Ty(LLVMCtx);
    Int32Ty = llvm::Type::getInt32Ty(LLVMCtx);
    Int64Ty = llvm::Type::getInt64Ty(LLVMCtx);
    HalfTy = llvm::Type::getHalfTy(LLVMCtx);
    BFloatTy = llvm::Type::getBFloatTy(LLVMCtx);
    FloatTy = llvm::Type::getFloatTy(LLVMCtx);
    DoubleTy = llvm::Type::getDoubleTy(LLVMCtx);
    PointerWidthInBits = Target.getPointerWidth(0);
    PointerAlignInBytes = toCharUnitsFromBits(Target.getPointerAlign(0)).getQuantity();
    SizeSizeInBytes = toCharUnitsFromBits(Target.getMaxPointerWidth()).getQuantity();
    IntAlignInBytes = toCharUnitsFromBits(Target.getIntAlign()).getQuantity();
    IntTy = llvm::IntegerType::get(LLVMCtx, Target.getIntWidth());
    IntPtrTy = llvm::IntegerType::get(LLVMCtx, Target.getMaxPointerWidth());
    Int8PtrTy = Int8Ty->getPointerTo(0);
    Int8PtrPtrTy = Int8PtrTy->getPointerTo(0);
    AllocaInt8PtrTy = Int8Ty->getPointerTo(Module->getDataLayout().getAllocaAddrSpace());

    // If debug info or coverage generation is enabled, create the CGDebugInfo
    // object.
//    if (CGOpts.getDebugInfo() != codegenoptions::NoDebugInfo ||
//            CGOpts.EmitGcovArcs || CGOpts.EmitGcovNotes)
//        DebugInfo.reset(new CGDebugInfo(*this)); TODO

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
    for (FuncDecl *Func : Node.getFunctions()) {
        GenFunction(Func);
    }
}

/**
 * GenStmt from VarDecl
 * @param Decl
 */
CodeGenGlobalVar *CodeGenModule::GenGlobalVar(GlobalVarDecl* VDecl) {
    // Check Value
    llvm::StringRef StrVal;
    if (VDecl->getExpr() && !VDecl->getExpr()->isEmpty()) {
        assert(VDecl->getExpr()->getGroup().size() == 1 &&
                       VDecl->getExpr()->getGroup().at(0)->getKind() == EXPR_VALUE && "Invalid Global Var value");
        ValueExpr *E = static_cast<ValueExpr *>(VDecl->getExpr()->getGroup().at(0));
        StrVal = E->getString();
    }

    CodeGenGlobalVar *CG = new CodeGenGlobalVar(this, VDecl->getName(), VDecl->getType(), StrVal, VDecl->isConstant());
    VDecl->setCodeGen(CG);
    return CG;
}

CodeGenFunction *CodeGenModule::GenFunction(FuncDecl *FDecl) {
    CodeGenFunction *CGF = new CodeGenFunction(this, FDecl->getName(), FDecl->getType(), FDecl->getHeader(), FDecl->getBody());
    FDecl->setCodeGen(CGF);
    return CGF;
}

CallInst *CodeGenModule::GenCall(FuncCall *Call) {
    assert(Call->getDecl() && "Declaration not resolved");

    const std::vector<FuncParam *> &Params = Call->getDecl()->getHeader()->getParams();
    llvm::SmallVector<llvm::Value *, 8> Args;
    for (FuncCallArg *Arg : Call->getArgs()) {

        switch (Arg->getValue()->getKind()) {

            case EXPR_VALUE: {
                ValueExpr *ValExp = static_cast<ValueExpr *>(Arg->getValue());
                Constant *C = GenValue(Arg->getType(), ValExp->getString());
                Args.push_back(C);
                break;
            }
            case EXPR_REF_VAR:
                // TODO
                break;
            case EXPR_REF_FUNC:
                break;
            case EXPR_GROUP:
                break;
            default:
                assert("Invalid Function Args");
        }
    }
    return Builder->CreateCall(Call->getDecl()->getCodeGen()->getFunction(), Args);
}

void CodeGenModule::GenStmt(Stmt * S) {
    switch (S->getKind()) {

        // Var Declaration
        case StmtKind::STMT_VAR_DECL: {
            VarDeclStmt *V = static_cast<VarDeclStmt *>(S);
            assert(V->getCodeGen() && "VarDeclStmt is not CodeGen initialized");
            if (V->getExpr()) {
                GenExpr(V->getType(), V->getExpr());
            }
            break;
        }

        // Var Assignment
        case STMT_VAR_ASSIGN: {
            VarStmt *V = static_cast<VarStmt *>(S);
            assert(!V->getExpr()->isEmpty() && "Var assign empty");
            llvm::Value *Val = GenExpr(V->getDecl()->getType(), V->getExpr());
            if (V->getDecl()->isGlobal()) {
                GlobalVarDecl *GV = static_cast<GlobalVarDecl *>(V->getDecl());
                GV->getCodeGen()->Store(Val);
            } else {
                VarDeclStmt *LV = static_cast<VarDeclStmt *>(V->getDecl());
                LV->getCodeGen()->Store(Val);
            }
            break;
        }
        case STMT_FUNC_CALL: {
            FuncCallStmt *FCall = static_cast<FuncCallStmt *>(S);
            GenCall(FCall->getCall());
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
            if (R->getExpr()->isEmpty()) {
                Builder->CreateRetVoid();
            } else {
                llvm::Value *V = GenExpr(R->getTop()->getType(), R->getExpr());
                Builder->CreateRet(V);
            }
            break;
    }
}

llvm::Type *CodeGenModule::GenType(const TypeBase *TyData) {
    llvm::Type *Ty = nullptr;
    llvm::Constant *Const = nullptr;
    GenTypeValue(TyData, Ty, Const, "");
    return Ty;
}

llvm::Constant *CodeGenModule::GenValue(const TypeBase *TyData, StringRef StrVal) {
    llvm::Type *Ty = nullptr;
    llvm::Constant *Const = nullptr;
    GenTypeValue(TyData, Ty, Const, StrVal);
    return Const;
}

void CodeGenModule::GenTypeValue(const TypeBase *TyData, llvm::Type *&Ty, llvm::Constant *&Const, StringRef StrVal) {
    // Check Type
    switch (TyData->getKind()) {

        case TYPE_VOID:
            Ty = VoidTy;
            break;
        case TYPE_INT:
            if (!StrVal.empty()) {
                uint64_t intVal = std::stoi(StrVal.str());
                Const = llvm::ConstantInt::get(Int32Ty, intVal, true);
            }
            Ty = Int32Ty;
            break;
        case TYPE_FLOAT:
            if (!StrVal.empty()) {
                Const = llvm::ConstantFP::get(FloatTy, StrVal);
            }
            Ty = FloatTy;
            break;
        case TYPE_BOOL:
            if (!StrVal.empty()) {
                if (StrVal.equals("true")) {
                    Const = llvm::ConstantInt::get(BoolTy, 1, false);
                } else if (StrVal.equals("false")) {
                    Const = llvm::ConstantInt::get(BoolTy, 0, false);
                } else {
                    // TODO error bad Bool value
                }
            }
            Ty = BoolTy;
            break;
        default:
            llvm_unreachable("Missing Var Type");
    }
}

llvm::Value *CodeGenModule::GenExpr(const TypeBase *Typ, GroupExpr *Expr) {
    for (auto *E : Expr->getGroup()) {
        switch (E->getKind()) {

            case EXPR_VALUE: {
                ValueExpr *ValEx = static_cast<ValueExpr *>(E);
                return GenValue(Typ, ValEx->getString());
            }
            case EXPR_OPERATOR:
                return nullptr;
            case EXPR_REF_VAR: {
                VarRefExpr *RefExp = static_cast<VarRefExpr *>(E);
                assert(RefExp->getRef() && "Missing Ref");
                VarDecl *VDecl = RefExp->getRef()->getDecl();
                if (VDecl->isGlobal()) {
                    return static_cast<GlobalVarDecl *>(VDecl)->getCodeGen()->getGlobalVar();
                }
                return static_cast<VarDeclStmt *>(VDecl)->getCodeGen()->get();
            }
            case EXPR_REF_FUNC: {
                FuncCallExpr *RefExp = static_cast<FuncCallExpr *>(E);
                assert(RefExp->getRef() && "Missing Ref");
                return GenCall(RefExp->getRef());
            }
            case EXPR_GROUP:
                return GenExpr(Typ, static_cast<GroupExpr *>(E));
        }
    }
    llvm_unreachable("Missing Expr Type");
}

CodeGenModule::~CodeGenModule() {
    delete Module;
    delete Builder;
}
