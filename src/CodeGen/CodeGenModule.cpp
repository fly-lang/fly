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
#include "CodeGen/CodeGenClass.h"
#include "CodeGen/CodeGenGlobalVar.h"
#include "CodeGen/CodeGenVar.h"
#include "CodeGen/CodeGenExpr.h"
#include "Sema/SemaBuilder.h"
#include "AST/ASTImport.h"
#include "AST/ASTNode.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTCall.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTClassVar.h"
#include "AST/ASTClassFunction.h"
#include "AST/ASTFunction.h"
#include "AST/ASTParams.h"
#include "AST/ASTBlock.h"
#include "AST/ASTIfBlock.h"
#include "AST/ASTSwitchBlock.h"
#include "AST/ASTWhileBlock.h"
#include "AST/ASTForBlock.h"
#include "AST/ASTValue.h"
#include "AST/ASTVarAssign.h"
#include "AST/ASTVarRef.h"
#include "Basic/Debug.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Value.h"

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
    if (CGGV->getPointer()) { // Pointer is the GlobalVar, if is nullptr CodeGenGlobalVar is nullptr
        GlobalVar->setCodeGen(CGGV);
        return CGGV;
    }
    return nullptr; // Error occurs
}

CodeGenFunction *CodeGenModule::GenFunction(ASTFunction *Function, bool isExternal) {
    FLY_DEBUG_MESSAGE("CodeGenModule", "AddFunction",
                      "Function=" << Function->str() << ", isExternal=" << isExternal);
    CodeGenFunction *CGF = new CodeGenFunction(this, Function, isExternal);
    CGF->Create();
    Function->setCodeGen(CGF);
    return CGF;
}

CodeGenClass *CodeGenModule::GenClass(ASTClass *Class, bool isExternal) {
    FLY_DEBUG_MESSAGE("CodeGenModule", "AddFunction",
                      "Class=" << Class->str() << ", isExternal=" << isExternal);
    CodeGenClass *CGC = new CodeGenClass(this, Class, isExternal);
    Class->setCodeGen(CGC);
    CGC->Generate();
    return CGC;
}

llvm::Type *CodeGenModule::GenType(const ASTType *Type) {
    FLY_DEBUG("CodeGenModule", "GenType");
    // Check Type
    switch (Type->getKind()) {

        case ASTTypeKind::TYPE_VOID:
            return VoidTy;
        case ASTTypeKind::TYPE_BOOL:
            return BoolTy;
        case ASTTypeKind::TYPE_BYTE:
            return Int8Ty;
        case ASTTypeKind::TYPE_USHORT:
        case ASTTypeKind::TYPE_SHORT:
            return Int16Ty;
        case ASTTypeKind::TYPE_UINT:
        case ASTTypeKind::TYPE_INT:
            return Int32Ty;
        case ASTTypeKind::TYPE_ULONG:
        case ASTTypeKind::TYPE_LONG:
            return Int64Ty;
        case ASTTypeKind::TYPE_FLOAT:
            return FloatTy;
        case ASTTypeKind::TYPE_DOUBLE:
            return DoubleTy;
        case ASTTypeKind::TYPE_ARRAY: {
            return GenArrayType((ASTArrayType *) Type);
        }
        case ASTTypeKind::TYPE_CLASS: {
            ASTClass *Class = ((ASTClassType *) Type)->getDef();
            assert(Class && "Unreferenced Class Type");
            assert(Class->getCodeGen() && "Empty Class CodeGen");
            return Class->getCodeGen()->getType();
        }

    }
    assert(0 && "Unknown Var Type Kind");
}

llvm::ArrayType *CodeGenModule::GenArrayType(const ASTArrayType *ArrayType) {
    llvm::Type *SubType = GenType(ArrayType->getType());
    if (ArrayType->getSize()->getExprKind() == ASTExprKind::EXPR_VALUE) {
        ASTValueExpr *SizeExpr = (ASTValueExpr *) ArrayType->getSize();
        ASTIntegerValue &SizeValue = (ASTIntegerValue &) SizeExpr->getValue();
        return llvm::ArrayType::get(SubType, SizeValue.getValue());
    }

    assert("Array Size error");
}

llvm::Constant *CodeGenModule::GenDefaultValue(const ASTType *Type, llvm::Type *Ty) {
    FLY_DEBUG("CodeGenModule", "GenDefaultValue");
    assert(Type->getKind() != ASTTypeKind::TYPE_VOID && "No default value for Void Type");
    switch (Type->getKind()) {
        case ASTTypeKind::TYPE_BOOL:
            return llvm::ConstantInt::get(BoolTy, 0, false);
        case ASTTypeKind::TYPE_BYTE:
            return llvm::ConstantInt::get(Int8Ty, 0, false);
        case ASTTypeKind::TYPE_USHORT:
            return llvm::ConstantInt::get(Int32Ty, 0, false);
        case ASTTypeKind::TYPE_SHORT:
            return llvm::ConstantInt::get(Int32Ty, 0, true);
        case ASTTypeKind::TYPE_UINT:
            return llvm::ConstantInt::get(Int32Ty, 0, false);
        case ASTTypeKind::TYPE_INT:
            return llvm::ConstantInt::get(Int32Ty, 0, true);
        case ASTTypeKind::TYPE_ULONG:
            return llvm::ConstantInt::get(Int64Ty, 0, false);
        case ASTTypeKind::TYPE_LONG:
            return llvm::ConstantInt::get(Int64Ty, 0, true);
        case ASTTypeKind::TYPE_FLOAT:
            return llvm::ConstantFP::get(FloatTy, 0.0);
        case ASTTypeKind::TYPE_DOUBLE:
            return llvm::ConstantFP::get(DoubleTy, 0.0);
        case ASTTypeKind::TYPE_ARRAY:
            return ConstantAggregateZero::get(Ty);
        case ASTTypeKind::TYPE_CLASS:
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
        case ASTTypeKind::TYPE_BOOL:
            return llvm::ConstantInt::get(BoolTy, ((ASTBoolValue *)Val)->getValue(), false);
        case ASTTypeKind::TYPE_BYTE:
            return llvm::ConstantInt::get(Int8Ty, ((ASTIntegerValue *) Val)->getValue(), false);
        case ASTTypeKind::TYPE_USHORT:
            return llvm::ConstantInt::get(Int16Ty, ((ASTIntegerValue *) Val)->getValue(), false);
        case ASTTypeKind::TYPE_SHORT:
            return llvm::ConstantInt::get(Int16Ty, ((ASTIntegerValue *) Val)->getValue(), true);
        case ASTTypeKind::TYPE_UINT:
            return llvm::ConstantInt::get(Int32Ty, ((ASTIntegerValue *) Val)->getValue(), false);
        case ASTTypeKind::TYPE_INT:
            return llvm::ConstantInt::get(Int32Ty, ((ASTIntegerValue *) Val)->getValue(), true);
        case ASTTypeKind::TYPE_ULONG:
            return llvm::ConstantInt::get(Int64Ty, ((ASTIntegerValue *) Val)->getValue(), false);
        case ASTTypeKind::TYPE_LONG:
            return llvm::ConstantInt::get(Int64Ty, ((ASTIntegerValue *) Val)->getValue(), true);
        case ASTTypeKind::TYPE_FLOAT:
            return llvm::ConstantFP::get(FloatTy, ((ASTFloatingValue *) Val)->getValue());
        case ASTTypeKind::TYPE_DOUBLE:
            return llvm::ConstantFP::get(DoubleTy, ((ASTFloatingValue *) Val)->getValue());
        case ASTTypeKind::TYPE_ARRAY: {
            llvm::ArrayType *ArrType = GenArrayType((ASTArrayType *) Type);
            std::vector<llvm::Constant *> Values;
            for (ASTValue *Value : ((ASTArrayValue *) Val)->getValues()) {
                llvm::Constant * V = GenValue(((ASTArrayType *) Type)->getType(), Value);
                Values.push_back(V);
            }
            return llvm::ConstantArray::get(ArrType, makeArrayRef(Values));
        }
        case ASTTypeKind::TYPE_CLASS:
            break;
        case ASTTypeKind::TYPE_VOID:
            // FIXME
            break;
    }
    assert(0 && "Unknown Type");
}

void CodeGenModule::GenStmt(llvm::Function *Fn, ASTStmt * Stmt) {
    FLY_DEBUG("CodeGenModule", "GenStmt");
    switch (Stmt->getKind()) {

        // Var Declaration
        case ASTStmtKind::STMT_VAR_DEFINE: {
            ASTLocalVar *LocalVar = (ASTLocalVar *) Stmt;
            assert(LocalVar->getCodeGen() && "LocalVar is not CodeGen initialized");
            if (LocalVar->getExpr()) {
                bool NoStore = false;
                llvm::Value *V = GenExpr(Fn, LocalVar->getType(), LocalVar->getExpr(), NoStore);
                if (!NoStore) {
                    LocalVar->getCodeGen()->Store(V);
                }
            }
            break;
        }

            // Var Assignment
        case ASTStmtKind::STMT_VAR_ASSIGN: {
            ASTVarAssign *VarAssign = (ASTVarAssign *) Stmt;
            assert(VarAssign->getExpr() && "Expr Mandatory in assignment");
            ASTVarRef *VarRef = VarAssign->getVarRef();
            bool NoStore = false;
            llvm::Value *V = GenExpr(Fn, VarRef->getDef()->getType(), VarAssign->getExpr(), NoStore);
            GenVarRef(VarRef);
            if (!NoStore) {
                VarRef->getDef()->getCodeGen()->Store(V);
            }
            break;
        }
        case ASTStmtKind::STMT_EXPR: {
            ASTExprStmt *ExprStmt = (ASTExprStmt *) Stmt;
            GenExpr(Fn, ExprStmt->getExpr()->getType(), ExprStmt->getExpr());
            break;
        }
        case ASTStmtKind::STMT_BLOCK: {
            ASTBlock *Block = (ASTBlock *) Stmt;
            switch (Block->getBlockKind()) {
                case ASTBlockKind::BLOCK:
                    GenBlock(Fn, Block->getContent());
                    break;
                case ASTBlockKind::BLOCK_IF:
                    GenIfBlock(Fn, (ASTIfBlock *)Block);
                    break;
                case ASTBlockKind::BLOCK_ELSIF:
                case ASTBlockKind::BLOCK_ELSE:
                    // All done into BLOCK_STMT_IF
                    break;
                case ASTBlockKind::BLOCK_SWITCH:
                    GenSwitchBlock(Fn, (ASTSwitchBlock *)Block);
                    break;
                case ASTBlockKind::BLOCK_SWITCH_CASE:
                case ASTBlockKind::BLOCK_SWITCH_DEFAULT:
                    // All done into BLOCK_STMT_SWITCH
                    break;
                case ASTBlockKind::BLOCK_FOR:
                    GenForBlock(Fn, (ASTForBlock *)Block);
                    break;
                case ASTBlockKind::BLOCK_WHILE:
                    GenWhileBlock(Fn, (ASTWhileBlock *)Block);
                    break;
            }
            break;
        }
        case ASTStmtKind::STMT_BREAK:
            // TODO
            break;
        case ASTStmtKind::STMT_CONTINUE:
            // TODO
            break;
        case ASTStmtKind::STMT_RETURN:
            ASTReturn *Return = (ASTReturn *) Stmt;
            if (Return->getParent()->getKind() == ASTStmtKind::STMT_BLOCK) {
                if (((ASTBlock *) Return->getParent())->getTop()->getType()->getKind() == ASTTypeKind::TYPE_VOID) {
                    if (Return->getExpr() == nullptr) {
                        Builder->CreateRetVoid();
                    } else {
                        Diag(Return->getExpr()->getLocation(), diag::err_invalid_return_type);
                    }
                } else {
                    llvm::Value *V = GenExpr(Fn, ((ASTBlock *) Return->getParent())->getTop()->getType(), Return->getExpr());
                    Builder->CreateRet(V);
                }
            }
            break;
    }
}

llvm::Value *CodeGenModule::GenInstance(ASTReference *Reference) {
    if (Reference == nullptr)
        return nullptr;

    if (Reference->isCall()) {
        ASTFunctionBase* Instance = ((ASTCall *) Reference)->getDef();
        bool noStore = false;
        return GenCall(Instance->getCodeGen()->getFunction(), (ASTCall *) Reference, noStore);
    } else {
        ASTVar *Instance = ((ASTVarRef *) Reference)->getDef();
        GenVarRef((ASTVarRef *) Reference);
        return Instance->getCodeGen()->getValue();
    }
}

void CodeGenModule::GenVarRef(ASTVarRef *VarRef) {
    if (VarRef->getDef() == nullptr) {
        Diag(VarRef->getLocation(), diag::err_unref_var);
        return;
    }
    if (VarRef->getDef()->getVarKind() == ASTVarKind::VAR_CLASS) {
        llvm::Value *V = GenInstance(VarRef->getInstance()); // Set Instance into CodeGen
        ((ASTClassVar *) VarRef->getDef())->getCodeGen()->Init(V);
    }
}

llvm::Value *CodeGenModule::GenCall(llvm::Function *Fn, ASTCall *Call, bool &NoStore) {
    FLY_DEBUG_MESSAGE("CodeGenModule", "GenCall",
                      "Call=" << Call->str());
    // Check if Func is declared
    if (Call->getDef() == nullptr) {
        Diag(Call->getLocation(), diag::err_unref_call);
        return nullptr;
    }

    // The function arguments
    llvm::SmallVector<llvm::Value *, 8> Args;

    // Add Instance to Function Args
    Value *Instance = GenInstance(Call->getInstance());
    if (Call->getDef()->getKind() == ASTFunctionKind::CLASS_FUNCTION) {
        ASTClassFunction *Def = (ASTClassFunction *) Call->getDef();
        // Do not store Call return to the Instance
        if (Def->isConstructor())
            NoStore = true;

        // Set Instance
        if (!Def->isStatic()) {
            // add Class Instance to the args
            Args.push_back(Instance);
        }
    }

    // Add Call arguments to Function args
    const std::vector<ASTParam *> &Params = Call->getDef()->getParams()->getList();
    for (ASTArg *Arg : Call->getArgs()) {
        llvm::Value *V = GenExpr(Fn, Arg->getDef()->getType(), Arg->getExpr());
        Args.push_back(V);
    }

    // Add Function
    if (Call->getDef()->getKind() == ASTFunctionKind::CLASS_FUNCTION) {
        ASTClassFunction *Def = (ASTClassFunction *) Call->getDef();

        // Return Instance Pointer only on Constructor
        if (Def->isConstructor())
            return Instance;
    }

    CodeGenFunctionBase *CGF = Call->getDef()->getCodeGen();
    return Builder->CreateCall(CGF->getFunction(), Args);
}

llvm::Value *CodeGenModule::GenExpr(llvm::Function *Fn, const ASTType *Type, ASTExpr *Expr) {
    bool NoStore = false;
    return GenExpr(Fn, Type, Expr, NoStore);
}

llvm::Value *CodeGenModule::GenExpr(llvm::Function *Fn, const ASTType *Type, ASTExpr *Expr, bool &NoStore) {
    FLY_DEBUG("CodeGenModule", "GenExpr");
    CodeGenExpr *CGExpr = new CodeGenExpr(this, Fn, Expr, Type);
    NoStore = CGExpr->isNoStore();
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
    FLY_DEBUG("CodeGenModule", "GenIfBlock");
    ASTBoolType * BoolType = SemaBuilder::CreateBoolType(SourceLocation()); // used to force bool in condition expr

    // If Block
    llvm::Value *IfCond = GenExpr(Fn, BoolType, If->getCondition());
    llvm::BasicBlock *IfBB = llvm::BasicBlock::Create(LLVMCtx, "ifthen", Fn);

    // Create End block
    llvm::BasicBlock *EndBB = llvm::BasicBlock::Create(LLVMCtx, "endif", Fn);

    if (!If->getElseBlock()) {

        if (If->getElsifBlocks().empty()) { // If ...
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
            unsigned long Size = If->getElsifBlocks().size();
            for (unsigned long i = 0; i < If->getElsifBlocks().size(); i++) {
                llvm::BasicBlock *ElsifThenBB = llvm::BasicBlock::Create(LLVMCtx, "elsifthen", Fn, EndBB);

                llvm::BasicBlock *NextElsifBB;
                if (i == Size-1) { // is Last
                    NextElsifBB = EndBB;
                } else {
                    NextElsifBB = llvm::BasicBlock::Create(LLVMCtx, "elsif", Fn, EndBB);
                }
                ASTElsifBlock *Elsif = If->getElsifBlocks()[i];
                Builder->SetInsertPoint(ElsifBB);
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

        if (If->getElsifBlocks().empty()) { // If - Else
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
            unsigned long Size = If->getElsifBlocks().size();
            for (unsigned long i = 0; i < If->getElsifBlocks().size(); i++) {
                llvm::BasicBlock *ElsifThenBB = llvm::BasicBlock::Create(LLVMCtx, "elsifthen", Fn, ElseBB);

                llvm::BasicBlock *NextElsifBB;
                if (i == Size-1) { // is Last
                    NextElsifBB = ElseBB;
                } else {
                    NextElsifBB = llvm::BasicBlock::Create(LLVMCtx, "elsif", Fn, ElseBB);
                }
                ASTElsifBlock *Elsif = If->getElsifBlocks()[i];
                Builder->SetInsertPoint(ElsifBB);
                ASTBoolType * BoolType = SemaBuilder::CreateBoolType(SourceLocation());
                llvm::Value *ElsifCond = GenExpr(Fn, BoolType, Elsif->getCondition());
                Builder->CreateCondBr(ElsifCond, ElsifThenBB, NextElsifBB);

                GenBlock(Fn, Elsif->getContent(), ElsifThenBB);
                Builder->CreateBr(EndBB);

                ElsifBB = NextElsifBB;
            }
        }

        GenBlock(Fn, If->getElseBlock()->getContent(), ElseBB);
        Builder->CreateBr(EndBB);
    }

    // Continue insertions into End Branch
    Builder->SetInsertPoint(EndBB);
}

llvm::BasicBlock *CodeGenModule::GenElsifBlock(llvm::Function *Fn,
                                               llvm::BasicBlock *ElsifBB,
                                               std::vector<ASTElsifBlock *>::iterator &It) {
    FLY_DEBUG("CodeGenModule", "GenElsifBlock");
    ASTElsifBlock *&Elsif = *It;
    It++;
    if (*It == nullptr) {
        return ElsifBB;
    } else {
        Builder->SetInsertPoint(ElsifBB);
        ASTBoolType * BoolType = SemaBuilder::CreateBoolType(SourceLocation());
        llvm::Value *Cond = GenExpr(Fn, BoolType, Elsif->getCondition());
        llvm::BasicBlock *NextElsifBB = llvm::BasicBlock::Create(LLVMCtx, "elsif", Fn);
        Builder->CreateCondBr(Cond, ElsifBB, NextElsifBB);

        llvm::BasicBlock *ElsifThenBB = llvm::BasicBlock::Create(LLVMCtx, "elsifthen", Fn);
        GenBlock(Fn, Elsif->getContent(), ElsifThenBB);
        return GenElsifBlock(Fn, ElsifThenBB, It);
    }
}

void CodeGenModule::GenSwitchBlock(llvm::Function *Fn, ASTSwitchBlock *Switch) {
    FLY_DEBUG("CodeGenModule", "GenSwitchBlock");
    ASTIntType * IntType = SemaBuilder::CreateIntType(SourceLocation()); // used to force int in switch case expr valuation

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
    FLY_DEBUG("CodeGenModule", "GenForBlock");
    ASTBoolType * BoolType = SemaBuilder::CreateBoolType(SourceLocation()); // used to force bool in condition expr

    // Add to Current Block
    if (!For->isEmpty()) {
        GenBlock(Fn, For->getContent());
    }

    // Create Loop Block
    llvm::BasicBlock *LoopBB = LoopBB = llvm::BasicBlock::Create(LLVMCtx, "forloop", Fn);

    // Create Post Block
    llvm::BasicBlock *PostBB = nullptr;
    if (For->getPost() && !For->getPost()->isEmpty()) {
        PostBB = llvm::BasicBlock::Create(LLVMCtx, "forpost", Fn);
    }

    // Create End Block
    llvm::BasicBlock *EndBB = llvm::BasicBlock::Create(LLVMCtx, "endfor", Fn);

    // Add to Condition
    llvm::BasicBlock *CondBB = nullptr;
    if (For->getCondition()) {
        CondBB = llvm::BasicBlock::Create(LLVMCtx, "forcond", Fn, LoopBB);
        Builder->CreateBr(CondBB);

        // Create Condition
        Builder->SetInsertPoint(CondBB);
        Value *Cond = GenExpr(Fn, BoolType, For->getCondition());
        Builder->CreateCondBr(Cond, LoopBB, EndBB);
    } else {
        Builder->CreateBr(LoopBB);
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
    FLY_DEBUG("CodeGenModule", "GenWhileBlock");
    ASTBoolType * BoolType = SemaBuilder::CreateBoolType(SourceLocation()); // used to force bool in while condition expr

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
