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
#include "AST/ASTImport.h"
#include "AST/ASTNode.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTBlock.h"
#include "AST/ASTIfBlock.h"
#include "AST/ASTSwitchBlock.h"
#include "AST/ASTWhileBlock.h"
#include "AST/ASTForBlock.h"
#include "AST/ASTValue.h"
#include "Basic/Debug.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Value.h"
#include<iostream>
#include <regex>

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
    delete Builder;
}

DiagnosticBuilder CodeGenModule::Diag(const SourceLocation &Loc, unsigned DiagID) {
    return Diags.Report(Loc, DiagID);
}

llvm::Module *CodeGenModule::getModule() const {
    return Module.get();
}

llvm::Module *CodeGenModule::ReleaseModule() {
    return Module.release();
}

/**
 * GenStmt from VarDecl
 * @param GlobalVar
 * @param isExternal
 */
CodeGenGlobalVar *CodeGenModule::GenGlobalVar(ASTGlobalVar* GlobalVar, bool isExternal) {
    FLY_DEBUG_MESSAGE("CodeGenModule", "AddGlobalVar",
                      "GlobalVar=" << GlobalVar->str() << ", isExternal=" << isExternal);
    // Check Value
    CodeGenGlobalVar *CGGV = new CodeGenGlobalVar(this, GlobalVar, isExternal);
    if (CGGV->getPointer() != nullptr) { // Pointer is the GlobalVar, if nullptr Success = false
        GlobalVar->setCodeGen(CGGV);
        return CGGV;
    }
    return nullptr; // Error occurs
}

CodeGenFunction *CodeGenModule::GenFunction(ASTFunc *Func, bool isExternal) {
    FLY_DEBUG_MESSAGE("CodeGenModule", "AddFunction",
                      "Func=" << Func->str() << ", isExternal=" << isExternal);
    CodeGenFunction *CGF = new CodeGenFunction(this, Func, isExternal);
    Func->setCodeGen(CGF);
    return CGF;
}

CallInst *CodeGenModule::GenCall(llvm::Function *Fn, ASTFuncCall *Call) {
    FLY_DEBUG_MESSAGE("CodeGenModule", "GenCall",
                      "Call=" << Call->str());
    // Check if Func is declared
    if (Call->getDecl() == nullptr) {
        Diag(Call->getLocation(), diag::err_unref_call) << Call->getName();
        return nullptr;
    }

    const std::vector<ASTFuncParam *> &Params = Call->getDecl()->getHeader()->getParams();
    llvm::SmallVector<llvm::Value *, 8> Args;
    for (ASTCallArg *Arg : Call->getArgs()) {
        Value *V = GenExpr(Fn, Arg->getType(), Arg->getValue());
        Args.push_back(V);
    }
    CodeGenFunction *CGF = Call->getDecl()->getCodeGen();
    return Builder->CreateCall(CGF->getFunction(), Args);
}

void CodeGenModule::GenStmt(llvm::Function *Fn, ASTStmt * Stmt) {
    FLY_DEBUG("CodeGenModule", "GenStmt");
    switch (Stmt->getKind()) {

        // Var Declaration
        case StmtKind::STMT_VAR_DECL: {
            ASTLocalVar *LocalVar = static_cast<ASTLocalVar *>(Stmt);
            assert(LocalVar->getCodeGen() && "LocalVar is not CodeGen initialized");
            assert(LocalVar->getExpr() && "Expr Mandatory in declaration");
            llvm::Value *V = GenExpr(Fn, LocalVar->getType(), LocalVar->getExpr());
            LocalVar->getCodeGen()->Store(V);
            break;
        }

            // Var Assignment
        case STMT_VAR_ASSIGN: {
            ASTLocalVarRef *LocalVarRef = (ASTLocalVarRef *) Stmt;
            assert(LocalVarRef->getExpr() && "Expr Mandatory in assignment");
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
        case STMT_EXPR: {
            ASTExprStmt *ExprStmt = (ASTExprStmt *) Stmt;
            Value *V = GenExpr(Fn, ExprStmt->getExpr()->getType(), ExprStmt->getExpr());
            break;
        }
        case STMT_BLOCK: {
            ASTBlock *Block = (ASTBlock *) Stmt;
            switch (Block->getBlockKind()) {
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

llvm::Type *CodeGenModule::GenType(const ASTType *Type) {
    FLY_DEBUG("CodeGenFunction", "GenType");
    // Check Type
    switch (Type->getKind()) {

        case TYPE_VOID:
            return VoidTy;
        case TYPE_BOOL:
            return BoolTy;
        case TYPE_BYTE:
            return Int8Ty;
        case TYPE_USHORT:
        case TYPE_SHORT:
            return Int16Ty;
        case TYPE_UINT:
        case TYPE_INT:
            return Int32Ty;
        case TYPE_ULONG:
        case TYPE_LONG:
            return Int64Ty;
        case TYPE_FLOAT:
            return FloatTy;
        case TYPE_DOUBLE:
            return DoubleTy;
        case TYPE_ARRAY: {
            ASTArrayType *ArrType = (ASTArrayType *) Type;
            llvm::Type *SubType = GenType(ArrType->getType());
            if (ArrType->getSize()->getKind() == EXPR_VALUE) {
                ASTValueExpr *SizeExpr = (ASTValueExpr *) ArrType->getSize();
                assert(SizeExpr->getType()->isInteger());
                ASTIntegerValue &SizeValue = (ASTIntegerValue &) SizeExpr->getValue();
                return llvm::ArrayType::get(SubType, SizeValue.getValue());
            }
            // TODO
            //Value *Size = GenExpr(ArrType, ArrType->getSize());
            
        }

    }
    assert(0 && "Unknown Var Type Kind");
}

llvm::Constant *CodeGenModule::GenDefaultValue(const ASTType *Type, llvm::Type *Ty) {
    FLY_DEBUG("CodeGenFunction", "GenDefaultValue");
    assert(Type->getKind() != TYPE_VOID && "No default value for Void Type");
    switch (Type->getKind()) {
        case TYPE_BOOL:
            return llvm::ConstantInt::get(BoolTy, 0, false);
        case TYPE_BYTE:
            return llvm::ConstantInt::get(Int8Ty, 0, false);
        case TYPE_USHORT:
            return llvm::ConstantInt::get(Int32Ty, 0, false);
        case TYPE_SHORT:
            return llvm::ConstantInt::get(Int32Ty, 0, true);
        case TYPE_UINT:
            return llvm::ConstantInt::get(Int32Ty, 0, false);
        case TYPE_INT:
            return llvm::ConstantInt::get(Int32Ty, 0, true);
        case TYPE_ULONG:
            return llvm::ConstantInt::get(Int64Ty, 0, false);
        case TYPE_LONG:
            return llvm::ConstantInt::get(Int64Ty, 0, true);
        case TYPE_FLOAT:
            return llvm::ConstantFP::get(FloatTy, 0.0);
        case TYPE_DOUBLE:
            return llvm::ConstantFP::get(DoubleTy, 0.0);
        case TYPE_ARRAY:
            return ConstantAggregateZero::get(Ty);
        case TYPE_CLASS:
            return nullptr; // TODO
    }
    assert(0 && "Unknown Type");
}

/**
 * Generate a LLVM Constant Value
 * @param Type is the parsed ASTType
 * @param Val need to be correctly configured or you need to call GenDefaultValue()
 * @return
 */
llvm::Constant *CodeGenModule::GenValue(const ASTType *Type, const ASTValue *Val) {
    FLY_DEBUG("CodeGenModule", "GenValue");
    assert(Type && "Type has to be not empty");
    assert(Val && "Value has to be not empty");

    //TODO value conversion from Val->getType() to TypeBase (if are different)
    
    switch (Type->getKind()) {

        case TYPE_BOOL:
            return ((ASTBoolValue *)Val)->getValue() ? 
                llvm::ConstantInt::get(BoolTy, 1, false) :
                llvm::ConstantInt::get(BoolTy, 0, false);

        case TYPE_BYTE: {
            uint64_t UIntVal = ((ASTIntegerValue *) Val)->getValue();
            if (UIntVal > MAX_BYTE) {
                Diag(Type->getLocation(), diag::err_value_max_overflow) << "byte";
                UIntVal = MAX_BYTE;
            }
            if (UIntVal < MIN_BYTE) {
                Diag(Type->getLocation(), diag::err_value_min_underflow) << "byte";
                UIntVal = MIN_BYTE;
            }
            return llvm::ConstantInt::get(Int8Ty, UIntVal, false);
        }

        case TYPE_USHORT: {
            uint64_t UIntVal = ((ASTIntegerValue *) Val)->getValue();
            if (UIntVal > MAX_USHORT) {
                Diag(Type->getLocation(), diag::err_value_max_overflow) << "ushort";
                UIntVal = MAX_USHORT;
            }
            if (UIntVal < MIN_USHORT) {
                Diag(Type->getLocation(), diag::err_value_min_underflow) << "ushort";
                UIntVal = MIN_USHORT;
            }
            return llvm::ConstantInt::get(Int16Ty, UIntVal, false);
        }

        case TYPE_SHORT: {
            uint64_t UIntVal = ((ASTIntegerValue *) Val)->getValue();
            if (UIntVal > MAX_SHORT) {
                Diag(Type->getLocation(), diag::err_value_max_overflow) << "short";
                UIntVal = MAX_SHORT;
            }
            if (UIntVal < MIN_SHORT) {
                Diag(Type->getLocation(), diag::err_value_min_underflow) << "short";
                UIntVal = MIN_SHORT;
            }
            return llvm::ConstantInt::get(Int16Ty, UIntVal, true);
        }

        case TYPE_UINT: {
            uint64_t UIntVal = ((ASTIntegerValue *) Val)->getValue();
            if (UIntVal > MAX_UINT) {
                Diag(Type->getLocation(), diag::err_value_max_overflow) << "uint";
                UIntVal = MAX_UINT;
            }
            if (UIntVal < MIN_UINT) {
                Diag(Type->getLocation(), diag::err_value_min_underflow) << "uint";
                UIntVal = MIN_UINT;
            }
            return llvm::ConstantInt::get(Int32Ty, UIntVal, false);
        }

        case TYPE_INT: {
            uint64_t UIntVal = ((ASTIntegerValue *) Val)->getValue();
            if (UIntVal > MAX_INT) {
                Diag(Type->getLocation(), diag::err_value_max_overflow) << "int";
                UIntVal = MAX_INT;
            }
            if (UIntVal < MIN_INT) {
                Diag(Type->getLocation(), diag::err_value_min_underflow) << "int";
                UIntVal = MIN_INT;
            }
            return llvm::ConstantInt::get(Int32Ty, UIntVal, true);
        }

        case TYPE_ULONG: {
            uint64_t UIntVal = ((ASTIntegerValue *) Val)->getValue();
            if (UIntVal > MAX_ULONG) {
                Diag(Type->getLocation(), diag::err_value_max_overflow) << "ulong";
                UIntVal = MAX_ULONG;
            }
            if (UIntVal < MIN_ULONG) {
                Diag(Type->getLocation(), diag::err_value_min_underflow) << "ulong";
                UIntVal = MIN_ULONG;
            }
            return llvm::ConstantInt::get(Int64Ty, UIntVal, false);
        }

        case TYPE_LONG: {
            uint64_t UIntVal = ((ASTIntegerValue *) Val)->getValue();
            if (UIntVal > MAX_LONG) {
                Diag(Type->getLocation(), diag::err_type_invalid_type) << "long";
                UIntVal = MAX_LONG;
            }
            if (UIntVal < MIN_LONG) {
                Diag(Type->getLocation(), diag::err_value_min_underflow) << "long";
                UIntVal = MIN_LONG;
            }
            return llvm::ConstantInt::get(Int64Ty, UIntVal, true);
        }

        case TYPE_FLOAT:
            return llvm::ConstantFP::get(FloatTy, ((ASTFloatingValue *) Val)->getValue());

        case TYPE_DOUBLE:
            return llvm::ConstantFP::get(DoubleTy, ((ASTFloatingValue *) Val)->getValue());

        case TYPE_CLASS:
            break;
    }
    assert(0 && "Unknown Type");
}

llvm::Value *CodeGenModule::GenExpr(llvm::Function *Fn, const ASTType *Type, ASTExpr *Expr) {
    FLY_DEBUG("CodeGenFunction", "GenExpr");
    CodeGenExpr *CGExpr = new CodeGenExpr(this, Fn, (ASTExpr *)Expr, Type);
    return CGExpr->getValue();
}

void CodeGenModule::GenBlock(llvm::Function *Fn, const std::vector<ASTStmt *> &Content, llvm::BasicBlock *BB) {
    FLY_DEBUG("CodeGenModule", "GenBlock");
    if (BB) Builder->SetInsertPoint(BB);
    for (ASTStmt *Stmt : Content) {
        GenStmt(Fn, Stmt);
    }
}

void CodeGenModule::GenIfBlock(llvm::Function *Fn, ASTIfBlock *If) {
    FLY_DEBUG("CodeGenFunction", "GenIfBlock");
    ASTBoolType * BoolType = new ASTBoolType(SourceLocation()); // used to force bool in condition expr

    // If Block
    llvm::Value *IfCond = GenExpr(Fn, BoolType, If->getCondition());
    llvm::BasicBlock *IfBB = llvm::BasicBlock::Create(LLVMCtx, "ifthen", Fn);

    // Create End block
    llvm::BasicBlock *EndBB = llvm::BasicBlock::Create(LLVMCtx, "endif", Fn);

    if (If->getElse() == nullptr) {

        if (If->getElsif().empty()) { // If ...
            Builder->CreateCondBr(IfCond, IfBB, EndBB);
            GenBlock(Fn, If->getContent(), IfBB);
            Builder->CreateBr(EndBB);
        } else { // If - elsif ...
            llvm::BasicBlock *ElsifBB = llvm::BasicBlock::Create(LLVMCtx, "elsif", Fn, EndBB);
            Builder->CreateCondBr(IfCond, IfBB, ElsifBB);

            // Start if-then
            GenBlock(Fn, If->getContent(), IfBB);
            Builder->CreateBr(EndBB);

            // Create Elsif Blocks
            unsigned long Size = If->getElsif().size();
            for (unsigned long i = 0; i < If->getElsif().size(); i++) {
                llvm::BasicBlock *ElsifThenBB = llvm::BasicBlock::Create(LLVMCtx, "elsifthen", Fn, EndBB);

                llvm::BasicBlock *NextElsifBB;
                if (i == Size-1) { // is Last
                    NextElsifBB = EndBB;
                } else {
                    NextElsifBB = llvm::BasicBlock::Create(LLVMCtx, "elsif", Fn, EndBB);
                }
                ASTElsifBlock *Elsif = If->getElsif()[i];
                Builder->SetInsertPoint(ElsifBB);
                ASTBoolType * BoolType = new ASTBoolType(SourceLocation());
                llvm::Value *ElsifCond = GenExpr(Fn, BoolType, Elsif->getCondition());
                Builder->CreateCondBr(ElsifCond, ElsifThenBB, NextElsifBB);

                GenBlock(Fn, Elsif->getContent(), ElsifThenBB);
                Builder->CreateBr(EndBB);

                ElsifBB = NextElsifBB;
            }
        }

    } else {

        // Create Else block
        llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(LLVMCtx, "else", Fn, EndBB);

        if (If->getElsif().empty()) { // If - Else
            Builder->CreateCondBr(IfCond, IfBB, ElseBB);
            GenBlock(Fn, If->getContent(), IfBB);
            Builder->CreateBr(EndBB);
        } else { // If - Elsif - Else
            llvm::BasicBlock *ElsifBB = llvm::BasicBlock::Create(LLVMCtx, "elsif", Fn, ElseBB);
            Builder->CreateCondBr(IfCond, IfBB, ElsifBB);

            // Start if-then
            GenBlock(Fn, If->getContent(), IfBB);
            Builder->CreateBr(EndBB);

            // Create Elsif Blocks
            unsigned long Size = If->getElsif().size();
            for (unsigned long i = 0; i < If->getElsif().size(); i++) {
                llvm::BasicBlock *ElsifThenBB = llvm::BasicBlock::Create(LLVMCtx, "elsifthen", Fn, ElseBB);

                llvm::BasicBlock *NextElsifBB;
                if (i == Size-1) { // is Last
                    NextElsifBB = ElseBB;
                } else {
                    NextElsifBB = llvm::BasicBlock::Create(LLVMCtx, "elsif", Fn, ElseBB);
                }
                ASTElsifBlock *Elsif = If->getElsif()[i];
                Builder->SetInsertPoint(ElsifBB);
                ASTBoolType * BoolType = new ASTBoolType(SourceLocation());
                llvm::Value *ElsifCond = GenExpr(Fn, BoolType, Elsif->getCondition());
                Builder->CreateCondBr(ElsifCond, ElsifThenBB, NextElsifBB);

                GenBlock(Fn, Elsif->getContent(), ElsifThenBB);
                Builder->CreateBr(EndBB);

                ElsifBB = NextElsifBB;
            }
        }

        GenBlock(Fn, If->getElse()->getContent(), ElseBB);
        Builder->CreateBr(EndBB);
    }

    // Continue insertions into End Branch
    Builder->SetInsertPoint(EndBB);
}

llvm::BasicBlock *CodeGenModule::GenElsifBlock(llvm::Function *Fn,
                                               llvm::BasicBlock *ElsifBB,
                                               std::vector<ASTElsifBlock *>::iterator &It) {
    FLY_DEBUG("CodeGenFunction", "GenElsifBlock");
    ASTElsifBlock *&Elsif = *It;
    It++;
    if (*It == nullptr) {
        return ElsifBB;
    } else {
        Builder->SetInsertPoint(ElsifBB);
        ASTBoolType * BoolType = new ASTBoolType(SourceLocation());
        llvm::Value *Cond = GenExpr(Fn, BoolType, Elsif->getCondition());
        llvm::BasicBlock *NextElsifBB = llvm::BasicBlock::Create(LLVMCtx, "elsif", Fn);
        Builder->CreateCondBr(Cond, ElsifBB, NextElsifBB);

        llvm::BasicBlock *ElsifThenBB = llvm::BasicBlock::Create(LLVMCtx, "elsifthen", Fn);
        GenBlock(Fn, Elsif->getContent(), ElsifThenBB);
        return GenElsifBlock(Fn, ElsifThenBB, It);
    }
}

void CodeGenModule::GenSwitchBlock(llvm::Function *Fn, ASTSwitchBlock *Switch) {
    FLY_DEBUG("CodeGenFunction", "GenSwitchBlock");
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
    FLY_DEBUG("CodeGenFunction", "GenForBlock");
    ASTBoolType * BoolType = new ASTBoolType(SourceLocation()); // used to force bool in condition expr

    // Add to Current Block
    if (!For->isEmpty()) {
        GenBlock(Fn, For->getContent());
    }

    // Create Loop Block
    llvm::BasicBlock *LoopBB = LoopBB = llvm::BasicBlock::Create(LLVMCtx, "forloop", Fn);

    // Create Post Block
    llvm::BasicBlock *PostBB = nullptr;
    if (!For->getPost()->isEmpty()) {
        PostBB = llvm::BasicBlock::Create(LLVMCtx, "forpost", Fn);
    }

    // Create End Block
    llvm::BasicBlock *EndBB = llvm::BasicBlock::Create(LLVMCtx, "endfor", Fn);

    // Add to Condition
    llvm::BasicBlock *CondBB = nullptr;
    if (For->getCondition()->isEmpty()) {
        Builder->CreateBr(LoopBB);
    } else {
        CondBB = llvm::BasicBlock::Create(LLVMCtx, "forcond", Fn, LoopBB);
        Builder->CreateBr(CondBB);

        // Take Condition from Block
        ASTExprStmt *ExprStmt = (ASTExprStmt *)For->getCondition()->getContent()[0];
        ASTGroupExpr *CondExpr = (ASTGroupExpr *)ExprStmt->getExpr();

        // Create Condition
        Builder->SetInsertPoint(CondBB);
        Value *Cond = GenExpr(Fn, BoolType, CondExpr);
        Builder->CreateCondBr(Cond, LoopBB, EndBB);
    }

    // Add to Loop
    GenBlock(Fn, For->getLoop()->getContent(), LoopBB);
    if (PostBB) {
        Builder->CreateBr(PostBB);
    } else if (CondBB) {
        Builder->CreateBr(CondBB);
    } else {
        Builder->CreateBr(LoopBB);
    }

    // Add to Post
    if (PostBB) {
        GenBlock(Fn, For->getPost()->getContent(), PostBB);
        if (CondBB) {
            Builder->CreateBr(CondBB);
        } else {
            Builder->CreateBr(LoopBB);
        }
    }

    // Continue insertions into End Branch
    Builder->SetInsertPoint(EndBB);
}

void CodeGenModule::GenWhileBlock(llvm::Function *Fn, ASTWhileBlock *While) {
    FLY_DEBUG("CodeGenFunction", "GenWhileBlock");
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
