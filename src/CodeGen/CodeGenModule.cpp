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
#include "CodeGen/CodeGenLocalVar.h"
#include "CodeGen/CodeGenExpr.h"
#include "AST/ASTNode.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTBlock.h"
#include "AST/ASTOperatorExpr.h"
#include "AST/ASTIfBlock.h"
#include "AST/ASTSwitchBlock.h"
#include "AST/ASTWhileBlock.h"
#include "AST/ASTForBlock.h"
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
                    GenBlock(Block->getContent());
                    break;
                case BLOCK_STMT_IF:
                    GenIfBlock((ASTIfBlock *)Block);
                    break;
                case BLOCK_STMT_ELSIF:
                case BLOCK_STMT_ELSE:
                    // All done into BLOCK_STMT_IF
                    break;
                case BLOCK_STMT_SWITCH:
                    GenSwitchBlock((ASTSwitchBlock *)Block);
                    break;
                case BLOCK_STMT_CASE:
                case BLOCK_STMT_DEFAULT:
                    // All done into BLOCK_STMT_SWITCH
                    break;
                case BLOCK_STMT_FOR:
                    GenForBlock((ASTForBlock *)Block);
                    break;
                case BLOCK_STMT_WHILE:
                    GenWhileBlock((ASTWhileBlock *)Block);
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

void CodeGenModule::GenBlock(const std::vector<ASTStmt *> &Content, llvm::BasicBlock *BB) {
    if (BB) Builder->SetInsertPoint(BB);
    for (ASTStmt *Stmt : Content) {
        GenStmt(Stmt);
    }
}

void CodeGenModule::GenIfBlock(ASTIfBlock *If) {
    ASTBoolType * BoolType = new ASTBoolType(SourceLocation()); // used to force bool in condition expr

    // If Block
    llvm::Value *IfCond = GenExpr(BoolType, If->getCondition());
    llvm::BasicBlock *IfBR = llvm::BasicBlock::Create(LLVMCtx);

    // Create Elsif Blocks
    llvm::BasicBlock *ElsifBR = nullptr;
    if (!If->getElsif().empty()) {
        ElsifBR = GenElsifBlock(IfCond, IfBR, If, If->getElsif().begin());
    }

    // Create Else
    llvm::BasicBlock *EndBR;
    if (If->getElse() == nullptr) {

        // Create End Block
        EndBR = llvm::BasicBlock::Create(LLVMCtx);

        if (ElsifBR == nullptr) { // If ...
            Builder->CreateCondBr(IfCond, IfBR, EndBR);
            GenBlock(If->getContent(), IfBR);
        } else { // If - elsif ...
            GenBlock(((ASTElsifBlock *) If->getElsif().back())->getContent(), ElsifBR);
        }
        Builder->CreateBr(EndBR);
    } else {
        llvm::BasicBlock *ElseBR = llvm::BasicBlock::Create(LLVMCtx);

        // Create End Block
        EndBR = llvm::BasicBlock::Create(LLVMCtx);

        if (ElsifBR == nullptr) { // If - Else
            Builder->CreateCondBr(IfCond, IfBR, ElseBR);
        } else { // If - Elsif - Else
            Builder->CreateCondBr(IfCond, ElsifBR, ElseBR);
            GenBlock(((ASTElsifBlock *) If->getElsif().back())->getContent(), ElsifBR);
            Builder->CreateBr(EndBR);
        }

        GenBlock(If->getElse()->getContent(), ElseBR);
        Builder->CreateBr(EndBR);
    }

    // Continue insertions into End Branch
    Builder->SetInsertPoint(EndBR);
}

llvm::BasicBlock *CodeGenModule::GenElsifBlock(llvm::Value *Cond, llvm::BasicBlock *TrueBB, ASTIfBlock *TrueBlock,
                                               std::vector<ASTElsifBlock *>::iterator It) {
    ASTElsifBlock *Elsif = *It;
    llvm::BasicBlock *FalseBB = llvm::BasicBlock::Create(LLVMCtx);
    Builder->CreateCondBr(Cond, TrueBB, FalseBB);
    GenBlock(TrueBlock->getContent(), TrueBB);

    ASTBoolType * BoolType = new ASTBoolType(SourceLocation());
    llvm::Value *NextCond = GenExpr(BoolType, Elsif->getCondition());

    It++;
    if (*It == nullptr) {
        return FalseBB;
    }
    return GenElsifBlock(NextCond, FalseBB, Elsif, It);
}

void CodeGenModule::GenSwitchBlock(ASTSwitchBlock *Switch) {
    ASTIntType * IntType = new ASTIntType(SourceLocation()); // used to force int in switch case expr valuation

    // Create End Block
    llvm::BasicBlock *EndBR = llvm::BasicBlock::Create(LLVMCtx);

    // Create Expression evaluator for Switch
    llvm::Value *SwitchVal = GenExpr(IntType, Switch->getExpr());
    llvm::SwitchInst *Inst = Builder->CreateSwitch(SwitchVal, EndBR);

    // Create Cases
    unsigned long Size = Switch->getCases().size();

    llvm::BasicBlock *NextCaseBB = nullptr;
    for (int i=0; i < Size; i++) {
        ASTSwitchCaseBlock *Case = Switch->getCases()[i];
        llvm::Value *CaseVal = GenExpr(IntType, Case->getExpr());
        llvm::ConstantInt *CaseConst = llvm::cast<llvm::ConstantInt, llvm::Value>(CaseVal);
        llvm::BasicBlock *CaseBB = NextCaseBB == nullptr ? llvm::BasicBlock::Create(LLVMCtx) : NextCaseBB;
        Inst->addCase(CaseConst, CaseBB);
        GenBlock(Case->getContent(), CaseBB);

        // If there is a Next
        if (i + 1 < Size) {
            NextCaseBB = llvm::BasicBlock::Create(LLVMCtx);
            Builder->CreateBr(NextCaseBB);
        } else {
            Builder->CreateBr(EndBR);
        }
    }

    // Create Default
    if (Switch->getDefault()) {
        llvm::BasicBlock *DefaultBB;
        Inst->setDefaultDest(DefaultBB);
        GenBlock(Switch->getDefault()->getContent(), DefaultBB);
        Builder->CreateBr(EndBR);
    }

    // Continue insertions into End Branch
    Builder->SetInsertPoint(EndBR);
}

void CodeGenModule::GenForBlock(ASTForBlock *For) {
    ASTBoolType * BoolType = new ASTBoolType(SourceLocation()); // used to force bool in condition expr


    // Add to Current Block
    if (!For->getInit()->isEmpty()) {
        GenBlock(For->getInit()->getContent());
    }

    // Create Condition Block
    if (For->getCondition() == nullptr) {
        Diag(For->getCondition()->getLocation(), diag::err_for_condition_mandatory);
    }
    llvm::BasicBlock *CondBR = llvm::BasicBlock::Create(LLVMCtx);
    Builder->CreateBr(CondBR); // Start by positioning into condition

    // Create Loop Block
    llvm::BasicBlock *LoopBR = nullptr;
    if (!For->getLoop()->isEmpty()) {
        LoopBR = llvm::BasicBlock::Create(LLVMCtx);
    }

    // Create Post Block
    llvm::BasicBlock *PostBR = nullptr;
    if (!For->getPost()->isEmpty()) {
        PostBR = llvm::BasicBlock::Create(LLVMCtx);
    }

    // Create End Block
    llvm::BasicBlock *EndBR = llvm::BasicBlock::Create(LLVMCtx);
    
    // Add to Condition
    Value *Cond = GenExpr(BoolType, For->getCondition());
    Builder->CreateCondBr(Cond, LoopBR, EndBR);

    // Add to Loop
    GenBlock(For->getLoop()->getContent(), LoopBR);

    // Add to Post
    GenBlock(For->getPost()->getContent(), PostBR);
    Builder->CreateBr(CondBR);

    // Continue insertions into End Branch
    Builder->SetInsertPoint(EndBR);
}

void CodeGenModule::GenWhileBlock(ASTWhileBlock *While) {
    ASTBoolType * BoolType = new ASTBoolType(SourceLocation()); // used to force bool in while condition expr

    // Create Expression evaluator for While
    llvm::BasicBlock *CondBR = llvm::BasicBlock::Create(LLVMCtx);
    
    // Create Loop Block
    llvm::BasicBlock *LoopBR = llvm::BasicBlock::Create(LLVMCtx);

    // Create End Block
    llvm::BasicBlock *EndBR = llvm::BasicBlock::Create(LLVMCtx);
    
    Builder->CreateBr(CondBR); // goto Condition Branch
    
    // Condition Branch
    Builder->SetInsertPoint(CondBR);
    llvm::Value *Cond = GenExpr(BoolType, While->getCondition());
    Builder->CreateCondBr(Cond, LoopBR, EndBR); // iF condition is true goto Loop Branch else goto End Branch

    // The While Block
    GenBlock(While->getContent(), LoopBR);
    Builder->CreateBr(CondBR);

    // Continue insertions into End Branch
    Builder->SetInsertPoint(EndBR);
}
