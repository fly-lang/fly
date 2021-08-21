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

CodeGenModule::CodeGenModule(DiagnosticsEngine &Diags, llvm::StringRef Name, LLVMContext &LLVMCtx, TargetInfo &Target,
                             CodeGenOptions &CGOpts) :
        Diags(Diags),
        Target(Target),
        Module(new llvm::Module(Name, LLVMCtx)),
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

Module *CodeGenModule::getModule() const {
    return Module;
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
        CG = new CodeGenGlobalVar(this, VDecl->getName(), VDecl->getType(), &E->getValue(), VDecl->isConstant());
    } else {
        CG = new CodeGenGlobalVar(this, VDecl->getName(), VDecl->getType(), nullptr, VDecl->isConstant());
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

CallInst *CodeGenModule::GenCall(llvm::Function *Fn, ASTFuncCall *Call) {
    // Check if Func is declared
    if (Call->getDecl() == nullptr) {
        Diag(Call->getLocation(), diag::err_func_notfound);
        return nullptr;
    }

    const std::vector<ASTFuncParam *> &Params = Call->getDecl()->getHeader()->getParams();
    llvm::SmallVector<llvm::Value *, 8> Args;
    for (ASTFuncArg *Arg : Call->getArgs()) {

        Value *V = GenExpr(Fn, Arg->getType(), Arg->getValue());
        Args.push_back(V);
    }
    return Builder->CreateCall(Call->getDecl()->getCodeGen()->getFunction(), Args);
}

void CodeGenModule::GenStmt(llvm::Function *Fn, ASTStmt * Stmt) {
    switch (Stmt->getKind()) {

        // Var Declaration
        case StmtKind::STMT_VAR_DECL: {
            ASTLocalVar *LocalVar = static_cast<ASTLocalVar *>(Stmt);
            assert(LocalVar->getCodeGen() && "VarDeclStmt is not CodeGen initialized");
            assert(LocalVar->getExpr() && "Expr Mandatory in STMT_VAR_DECL");
            Value *V = GenExpr(Fn, LocalVar->getType(), LocalVar->getExpr());
            LocalVar->getCodeGen()->Store(V);
            break;
        }

        // Var Assignment
        case STMT_VAR_ASSIGN: {
            ASTLocalVarRef *LocalVarRef = static_cast<ASTLocalVarRef *>(Stmt);
            assert(LocalVarRef->getExpr() && "Var Expr is unset empty");
            llvm::Value *V = GenExpr(Fn, LocalVarRef->getDecl()->getType(), LocalVarRef->getExpr());
            if (LocalVarRef->getDecl()->isGlobal()) {
                ASTGlobalVar *GlobalVar = static_cast<ASTGlobalVar *>(LocalVarRef->getDecl());
                GlobalVar->getCodeGen()->Store(V);
            } else {
                ASTLocalVar *LocalVar = static_cast<ASTLocalVar *>(LocalVarRef->getDecl());
                LocalVar->getCodeGen()->Store(V);
            }
            break;
        }
        case STMT_FUNC_CALL: {
            ASTFuncCallStmt *FCall = static_cast<ASTFuncCallStmt *>(Stmt);
            GenCall(Fn, FCall->getCall());
            break;
        }
        case STMT_BLOCK: {
            ASTBlock *Block = static_cast<ASTBlock *>(Stmt);
            switch (Block->getBlockKind()) {
                // TODO
                case BLOCK_STMT:
                    GenBlock(Fn, Block->getContent());
                    break;
                case BLOCK_STMT_IF:
                    GenIfBlock(Fn, (ASTIfBlock *)Block);
                    break;
                case BLOCK_STMT_ELSIF:
                case BLOCK_STMT_ELSE:
                    // All done into BLOCK_STMT_IF
                    break;
                case BLOCK_STMT_SWITCH:
                    GenSwitchBlock(Fn, (ASTSwitchBlock *)Block);
                    break;
                case BLOCK_STMT_CASE:
                case BLOCK_STMT_DEFAULT:
                    // All done into BLOCK_STMT_SWITCH
                    break;
                case BLOCK_STMT_FOR:
                    GenForBlock(Fn, (ASTForBlock *)Block);
                    break;
                case BLOCK_STMT_WHILE:
                    GenWhileBlock(Fn, (ASTWhileBlock *)Block);
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
            ASTReturn *Return = (ASTReturn *) Stmt;
            if (Return->getTop()->getType()->getKind() == TYPE_VOID) {
                if (Return->getExpr() == nullptr) {
                    Builder->CreateRetVoid();
                } else {
                    Diag(Return->getExpr()->getLocation(), diag::err_invalid_return_type);
                }
            } else {
                llvm::Value *V = GenExpr(Fn, Return->getTop()->getType(), Return->getExpr());
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

llvm::Value *CodeGenModule::GenExpr(llvm::Function *Fn, const ASTType *Type, ASTExpr *Expr) {
    CodeGenExpr *CGExpr = new CodeGenExpr(this, Fn, Expr, Type);
    return CGExpr->getValue();
}

void CodeGenModule::GenBlock(llvm::Function *Fn, const std::vector<ASTStmt *> &Content, llvm::BasicBlock *BB) {
    if (BB) Builder->SetInsertPoint(BB);
    for (ASTStmt *Stmt : Content) {
        GenStmt(Fn, Stmt);
    }
}

void CodeGenModule::GenIfBlock(llvm::Function *Fn, ASTIfBlock *If) {
    ASTBoolType * BoolType = new ASTBoolType(SourceLocation()); // used to force bool in condition expr

    // If Block
    llvm::Value *IfCond = GenExpr(Fn, BoolType, If->getCondition());
    llvm::BasicBlock *IfBR = llvm::BasicBlock::Create(LLVMCtx, "ifthen", Fn);

    // Create Elsif Blocks
    llvm::BasicBlock *ElsifBR = nullptr;
    if (!If->getElsif().empty()) {
        ElsifBR = GenElsifBlock(Fn, IfCond, IfBR, If, If->getElsif().begin());
    }

    // Create Else
    llvm::BasicBlock *EndBR;
    if (If->getElse() == nullptr) {

        // Create End Block
        EndBR = llvm::BasicBlock::Create(LLVMCtx, "endif", Fn);

        if (ElsifBR == nullptr) { // If ...
            Builder->CreateCondBr(IfCond, IfBR, EndBR);
            GenBlock(Fn, If->getContent(), IfBR);
        } else { // If - elsif ...
            GenBlock(Fn, ((ASTElsifBlock *) If->getElsif().back())->getContent(), ElsifBR);
        }
        Builder->CreateBr(EndBR);
    } else {
        llvm::BasicBlock *ElseBR = llvm::BasicBlock::Create(LLVMCtx, "else", Fn);

        // Create End Block
        EndBR = llvm::BasicBlock::Create(LLVMCtx, "endif", Fn);

        if (ElsifBR == nullptr) { // If - Else
            Builder->CreateCondBr(IfCond, IfBR, ElseBR);
            GenBlock(Fn, If->getContent(), IfBR);
        } else { // If - Elsif - Else
            Builder->CreateCondBr(IfCond, ElsifBR, ElseBR);
            GenBlock(Fn, ((ASTElsifBlock *) If->getElsif().back())->getContent(), ElsifBR);
        }
        Builder->CreateBr(EndBR);

        GenBlock(Fn, If->getElse()->getContent(), ElseBR);
        Builder->CreateBr(EndBR);
    }

    // Continue insertions into End Branch
    Builder->SetInsertPoint(EndBR);
}

llvm::BasicBlock *CodeGenModule::GenElsifBlock(llvm::Function *Fn, llvm::Value *Cond, llvm::BasicBlock *TrueBB,
                                               ASTIfBlock *TrueBlock,
                                               std::vector<ASTElsifBlock *>::iterator It) {
    ASTElsifBlock *Elsif = *It;
    llvm::BasicBlock *FalseBB = llvm::BasicBlock::Create(LLVMCtx, "elsif", Fn);
    Builder->CreateCondBr(Cond, TrueBB, FalseBB);
    GenBlock(Fn, TrueBlock->getContent(), TrueBB);

    ASTBoolType * BoolType = new ASTBoolType(SourceLocation());
    llvm::Value *NextCond = GenExpr(Fn, BoolType, Elsif->getCondition());

    It++;
    if (*It == nullptr) {
        return FalseBB;
    }
    return GenElsifBlock(Fn, NextCond, FalseBB, Elsif, It);
}

void CodeGenModule::GenSwitchBlock(llvm::Function *Fn, ASTSwitchBlock *Switch) {
    ASTIntType * IntType = new ASTIntType(SourceLocation()); // used to force int in switch case expr valuation

    // Create End Block
    llvm::BasicBlock *EndBR = llvm::BasicBlock::Create(LLVMCtx, "endswitch", Fn);

    // Create Expression evaluator for Switch
    llvm::Value *SwitchVal = GenExpr(Fn, IntType, Switch->getExpr());
    llvm::SwitchInst *Inst = Builder->CreateSwitch(SwitchVal, EndBR);

    // Create Cases
    unsigned long Size = Switch->getCases().size();

    llvm::BasicBlock *NextCaseBB = nullptr;
    for (int i=0; i < Size; i++) {
        ASTSwitchCaseBlock *Case = Switch->getCases()[i];
        llvm::Value *CaseVal = GenExpr(Fn, IntType, Case->getExpr());
        llvm::ConstantInt *CaseConst = llvm::cast<llvm::ConstantInt, llvm::Value>(CaseVal);
        llvm::BasicBlock *CaseBB = NextCaseBB == nullptr ?
                llvm::BasicBlock::Create(LLVMCtx, "case", Fn, EndBR) : NextCaseBB;
        Inst->addCase(CaseConst, CaseBB);
        GenBlock(Fn, Case->getContent(), CaseBB);

        // If there is a Next
        if (i + 1 < Size) {
            NextCaseBB = llvm::BasicBlock::Create(LLVMCtx, "case", Fn, EndBR);
            Builder->CreateBr(NextCaseBB);
        } else {
            Builder->CreateBr(EndBR);
        }
    }

    // Create Default
    if (Switch->getDefault()) {
        llvm::BasicBlock *DefaultBB = llvm::BasicBlock::Create(LLVMCtx, "default", Fn, EndBR);
        Inst->setDefaultDest(DefaultBB);
        GenBlock(Fn, Switch->getDefault()->getContent(), DefaultBB);
        Builder->CreateBr(EndBR);
    }

    // Continue insertions into End Branch
    Builder->SetInsertPoint(EndBR);
}

void CodeGenModule::GenForBlock(llvm::Function *Fn, ASTForBlock *For) {
    ASTBoolType * BoolType = new ASTBoolType(SourceLocation()); // used to force bool in condition expr


    // Add to Current Block
    if (!For->getInit()->isEmpty()) {
        GenBlock(Fn, For->getInit()->getContent());
    }

    // Create Condition Block
    if (For->getCondition() == nullptr) {
        Diag(For->getCondition()->getLocation(), diag::err_for_condition_mandatory);
    }
    llvm::BasicBlock *CondBR = llvm::BasicBlock::Create(LLVMCtx, "forcond", Fn);
    Builder->CreateBr(CondBR); // Start by positioning into condition

    // Create Loop Block
    llvm::BasicBlock *LoopBR = nullptr;
    if (!For->getLoop()->isEmpty()) {
        LoopBR = llvm::BasicBlock::Create(LLVMCtx, "forloop", Fn);
    }

    // Create Post Block
    llvm::BasicBlock *PostBR = nullptr;
    if (!For->getPost()->isEmpty()) {
        PostBR = llvm::BasicBlock::Create(LLVMCtx, "forpost", Fn);
    }

    // Create End Block
    llvm::BasicBlock *EndBR = llvm::BasicBlock::Create(LLVMCtx, "endfor", Fn);
    
    // Add to Condition
    Value *Cond = GenExpr(Fn, BoolType, For->getCondition());
    Builder->CreateCondBr(Cond, LoopBR, EndBR);

    // Add to Loop
    GenBlock(Fn, For->getLoop()->getContent(), LoopBR);

    // Add to Post
    GenBlock(Fn, For->getPost()->getContent(), PostBR);
    Builder->CreateBr(CondBR);

    // Continue insertions into End Branch
    Builder->SetInsertPoint(EndBR);
}

void CodeGenModule::GenWhileBlock(llvm::Function *Fn, ASTWhileBlock *While) {
    ASTBoolType * BoolType = new ASTBoolType(SourceLocation()); // used to force bool in while condition expr

    // Create Expression evaluator for While
    llvm::BasicBlock *CondBR = llvm::BasicBlock::Create(LLVMCtx, "whilecond", Fn);
    
    // Create Loop Block
    llvm::BasicBlock *LoopBR = llvm::BasicBlock::Create(LLVMCtx, "whileloop", Fn);

    // Create End Block
    llvm::BasicBlock *EndBR = llvm::BasicBlock::Create(LLVMCtx, "whileend", Fn);
    
    Builder->CreateBr(CondBR); // goto Condition Branch
    
    // Condition Branch
    Builder->SetInsertPoint(CondBR);
    llvm::Value *Cond = GenExpr(Fn, BoolType, While->getCondition());
    Builder->CreateCondBr(Cond, LoopBR, EndBR); // iF condition is true goto Loop Branch else goto End Branch

    // The While Block
    GenBlock(Fn, While->getContent(), LoopBR);
    Builder->CreateBr(CondBR);

    // Continue insertions into End Branch
    Builder->SetInsertPoint(EndBR);
}
