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
#include "CodeGen/CodeGenGlobalVar.h"
#include "CodeGen/CodeGenVar.h"
#include "CodeGen/CodeGenExpr.h"
#include "AST/ASTNode.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTBlock.h"
#include "AST/ASTOperatorExpr.h"
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

    // TODO Add dependencies, Linker Options
}

CodeGenModule::~CodeGenModule() {
    delete Module;
    delete Builder;
}

DiagnosticBuilder CodeGenModule::Diag(const SourceLocation &Loc, unsigned DiagID) {
    return Diags.Report(Loc, DiagID);
}

/**
 * GenStmt from ASTContext
 */
bool CodeGenModule::Generate() {
    // Manage Top Decl
    Node.getGlobalVars().begin();
    for (const auto &V : Node.getGlobalVars()) {
        GenGlobalVar(V.getValue());
    }
    for (ASTFunc *F : Node.getFunctions()) {
        GenFunction(F);
    }
    return true;
}

/**
 * GenStmt from VarDecl
 * @param Decl
 */
CodeGenGlobalVar *CodeGenModule::GenGlobalVar(ASTGlobalVar* VDecl) {
    // Check Value
    CodeGenGlobalVar *CG;
    if (VDecl->getExpr()) {
        assert((VDecl->getExpr() == nullptr || VDecl->getExpr()->getKind() == EXPR_VALUE) && "Invalid Global Var value");
        ASTValueExpr *E = static_cast<ASTValueExpr *>(VDecl->getExpr());
        CG = new CodeGenGlobalVar(this, VDecl->getName(), VDecl->getType(), &E->getValue(),
                                                    VDecl->isConstant());
    } else {
        CG = new CodeGenGlobalVar(this, VDecl->getName(), VDecl->getType(), nullptr,
                                  VDecl->isConstant());
    }

    VDecl->setCodeGen(CG);
    return CG;
}

CodeGenFunction *CodeGenModule::GenFunction(ASTFunc *FDecl) {
    CodeGenFunction *CGF = new CodeGenFunction(this, FDecl->getName(), FDecl->getType(), FDecl->getHeader(), 
                                               FDecl->getBody());
    FDecl->setCodeGen(CGF);
    return CGF;
}

CallInst *CodeGenModule::GenCall(ASTFuncCall *Call) {
    // Check if Func is declared
    if (Call->getDecl() == nullptr) {
        Diag(Call->getLocation(), diag::err_func_notfound);
        return nullptr;
    }

    const std::vector<ASTFuncParam *> &Params = Call->getDecl()->getHeader()->getParams();
    llvm::SmallVector<llvm::Value *, 8> Args;
    for (ASTFuncArg *Arg : Call->getArgs()) {

        Value *V = GenExpr(Arg->getType(), Arg->getValue());
        Args.push_back(V);
    }
    return Builder->CreateCall(Call->getDecl()->getCodeGen()->getFunction(), Args);
}

void CodeGenModule::GenStmt(ASTStmt * S) {
    switch (S->getKind()) {

        // Var Declaration
        case StmtKind::STMT_VAR_DECL: {
            ASTLocalVar *V = static_cast<ASTLocalVar *>(S);
            assert(V->getCodeGen() && "VarDeclStmt is not CodeGen initialized");
            if (V->getExpr()) {
                GenExpr(V->getType(), V->getExpr());
            }
            break;
        }

        // Var Assignment
        case STMT_VAR_ASSIGN: {
            ASTLocalVarStmt *V = static_cast<ASTLocalVarStmt *>(S);
            assert(V->getExpr() && "Var Expr is unset empty");
            llvm::Value *Val = GenExpr(V->getDecl()->getType(), V->getExpr());
            if (V->getDecl()->isGlobal()) {
                ASTGlobalVar *GV = static_cast<ASTGlobalVar *>(V->getDecl());
                GV->getCodeGen()->Store(Val);
            } else {
                ASTLocalVar *LV = static_cast<ASTLocalVar *>(V->getDecl());
                LV->getCodeGen()->Store(Val);
            }
            break;
        }
        case STMT_FUNC_CALL: {
            ASTFuncCallStmt *FCall = static_cast<ASTFuncCallStmt *>(S);
            GenCall(FCall->getCall());
            break;
        }
        case STMT_BLOCK: {
            ASTBlock *Block = static_cast<ASTBlock *>(S);
            switch (Block->getBlockKind()) {
                // TODO
                case BLOCK_STMT:
                    for (ASTStmt *Stmt : Block->getContent()) {
                        GenStmt(Stmt);
                    }
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
            ASTReturn *R = (ASTReturn *) S;
            if (R->getExpr() == nullptr) {
                Builder->CreateRetVoid();
            } else {
                llvm::Value *V = GenExpr(R->getTop()->getType(), R->getExpr());
                Builder->CreateRet(V);
            }
            break;
    }
}

llvm::Type *CodeGenModule::GenType(const ASTType *Ty) {
    // Check Type
    switch (Ty->getKind()) {

        case TYPE_VOID:
            return VoidTy;
        case TYPE_INT:
            return Int32Ty;
        case TYPE_FLOAT:
            return FloatTy;
        case TYPE_BOOL:
            return BoolTy;
    }
    assert(0 && "Unknown Var Type Kind");
}

llvm::Constant *CodeGenModule::GenValue(const ASTType *Ty, const ASTValue *Val) {
    //TODO value conversion from Val->getType() to TypeBase (if are different)
    switch (Ty->getKind()) {

        case TYPE_VOID:
            Diag(Val->getType()->getLocation(), diag::err_void_value);
            return nullptr;
        case TYPE_INT:
            if (!Val->empty()) {
                uint64_t intVal = std::stoi(Val->str().str());
                return llvm::ConstantInt::get(Int32Ty, intVal, true);
            }
            return llvm::ConstantInt::get(Int32Ty, 0, false);;
        case TYPE_FLOAT:
            if (!Val->empty()) {
                return llvm::ConstantFP::get(FloatTy, Val->str());
            }
            return llvm::ConstantFP::get(FloatTy, 0);
        case TYPE_BOOL:
            if (Val->isTrue()) {
                return llvm::ConstantInt::get(BoolTy, 1, false);
            }
            return llvm::ConstantInt::get(BoolTy, 0, false);
        case TYPE_CLASS:
            break;
    }
    assert(0 && "Unknown Type");
}

llvm::Value *CodeGenModule::GenExpr(const ASTType *Type, ASTExpr *Expr) {
    CodeGenExpr *CGExpr = new CodeGenExpr(this, Expr, Type);
    return CGExpr->getValue();
}
