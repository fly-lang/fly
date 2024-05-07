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
#include "CodeGen/CodeGenEnum.h"
#include "CodeGen/CodeGenInstance.h"
#include "CodeGen/CodeGenHandle.h"
#include "CodeGen/CodeGenError.h"
#include "AST/ASTImport.h"
#include "AST/ASTNode.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTDeleteStmt.h"
#include "AST/ASTCall.h"
#include "AST/ASTGlobalVar.h"
#include "AST/ASTClassAttribute.h"
#include "AST/ASTClassMethod.h"
#include "AST/ASTFailStmt.h"
#include "AST/ASTFunction.h"
#include "AST/ASTHandleStmt.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTIfStmt.h"
#include "AST/ASTSwitchStmt.h"
#include "AST/ASTLoopStmt.h"
#include "AST/ASTValue.h"
#include "AST/ASTVarStmt.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTClass.h"
#include "AST/ASTEnum.h"
#include "AST/ASTExprStmt.h"
#include "Basic/Debug.h"
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

CodeGenEnum *CodeGenModule::GenEnum(ASTEnum *Enum, bool isExternal) {
    FLY_DEBUG_MESSAGE("CodeGenModule", "AddFunction",
                      "Class=" << Enum->str() << ", isExternal=" << isExternal);
    CodeGenEnum *CGE = new CodeGenEnum(this, Enum, isExternal);
    Enum->setCodeGen(CGE);
    CGE->Generate();
    return CGE;
}

llvm::Type *CodeGenModule::GenType(const ASTType *Type) {
    FLY_DEBUG("CodeGenModule", "GenType");
    // Check Type
    switch (Type->getKind()) {

        case ASTTypeKind::TYPE_VOID:
            return VoidTy;

        case ASTTypeKind::TYPE_BOOL:
            return BoolTy;

        case ASTTypeKind::TYPE_INTEGER: {
            ASTIntegerTypeKind IntegerTypeKind = ((ASTIntegerType *) Type)->getIntegerKind();
            switch (IntegerTypeKind) {
                case ASTIntegerTypeKind::TYPE_BYTE:
                    return Int8Ty;
                case ASTIntegerTypeKind::TYPE_USHORT:
                case ASTIntegerTypeKind::TYPE_SHORT:
                    return Int16Ty;
                case ASTIntegerTypeKind::TYPE_UINT:
                case ASTIntegerTypeKind::TYPE_INT:
                    return Int32Ty;
                case ASTIntegerTypeKind::TYPE_ULONG:
                case ASTIntegerTypeKind::TYPE_LONG:
                    return Int64Ty;
            }
        }

        case ASTTypeKind::TYPE_FLOATING_POINT: {
            ASTFloatingPointTypeKind FloatingPointTypeKind = ((ASTFloatingPointType *) Type)->getFloatingPointKind();
            switch (FloatingPointTypeKind) {
                case ASTFloatingPointTypeKind::TYPE_FLOAT:
                    return FloatTy;
                case ASTFloatingPointTypeKind::TYPE_DOUBLE:
                    return DoubleTy;
            }
        }

        case ASTTypeKind::TYPE_ARRAY: {
            return GenArrayType((ASTArrayType *) Type);
        }

        case ASTTypeKind::TYPE_STRING: {
            return llvm::ArrayType::get(Int8Ty, 0);
        }

        case ASTTypeKind::TYPE_IDENTITY: {
            ASTIdentityTypeKind IdentityTypeKind = ((ASTIdentityType *) Type)->getIdentityTypeKind();

            // Error: unreferenced
            if (IdentityTypeKind == ASTIdentityTypeKind::TYPE_NONE) {
                Diag(Type->getLocation(), diag::err_cg_unref_identity_type) << ((ASTIdentityType *) Type)->getName();
                return nullptr;
            }

            switch (IdentityTypeKind) {
                case ASTIdentityTypeKind::TYPE_CLASS: {
                    ASTClass *Class = (ASTClass *) ((ASTClassType *) Type)->getDef();
                    return Class->getCodeGen()->getType();
                }
                case ASTIdentityTypeKind::TYPE_ENUM: {
                    ASTEnum *Enum = (ASTEnum *) ((ASTEnumType *) Type)->getDef();
                    return Enum->getCodeGen()->getType();
                }
            }
        }
    }
    assert(0 && "Unknown Var Type Kind");
}

llvm::ArrayType *CodeGenModule::GenArrayType(const ASTArrayType *ArrayType) {
    llvm::Type *SubType = GenType(ArrayType->getType());
    if (ArrayType->getSize()->getExprKind() == ASTExprKind::EXPR_VALUE) {
        ASTValueExpr *SizeExpr = (ASTValueExpr *) ArrayType->getSize();
        ASTIntegerValue *SizeValue = (ASTIntegerValue *) SizeExpr->getValue();
        return llvm::ArrayType::get(SubType, SizeValue->getValue());
    }

    return nullptr;
    assert("Array Size error");
}

llvm::Constant *CodeGenModule::GenDefaultValue(const ASTType *Type, llvm::Type *Ty) {
    FLY_DEBUG("CodeGenModule", "GenDefaultValue");
    assert(Type->getKind() != ASTTypeKind::TYPE_VOID && "No default value for Void Type");
    switch (Type->getKind()) {

        // Bool
        case ASTTypeKind::TYPE_BOOL:
            return llvm::ConstantInt::get(BoolTy, 0, false);

        // Integer
        case ASTTypeKind::TYPE_INTEGER: {
            ASTIntegerType *IntegerType = (ASTIntegerType *) Type;
            switch (IntegerType->getIntegerKind()) {
                case ASTIntegerTypeKind::TYPE_BYTE:
                    return llvm::ConstantInt::get(Int8Ty, 0, false);
                case ASTIntegerTypeKind::TYPE_USHORT:
                    return llvm::ConstantInt::get(Int32Ty, 0, false);
                case ASTIntegerTypeKind::TYPE_SHORT:
                    return llvm::ConstantInt::get(Int32Ty, 0, true);
                case ASTIntegerTypeKind::TYPE_UINT:
                    return llvm::ConstantInt::get(Int32Ty, 0, false);
                case ASTIntegerTypeKind::TYPE_INT:
                    return llvm::ConstantInt::get(Int32Ty, 0, true);
                case ASTIntegerTypeKind::TYPE_ULONG:
                    return llvm::ConstantInt::get(Int64Ty, 0, false);
                case ASTIntegerTypeKind::TYPE_LONG:
                    return llvm::ConstantInt::get(Int64Ty, 0, true);
            }
        }

        // Floating Point
        case ASTTypeKind::TYPE_FLOATING_POINT: {
            ASTFloatingPointType *FloatingPointType = (ASTFloatingPointType *) Type;
            switch (FloatingPointType->getFloatingPointKind()) {
                case ASTFloatingPointTypeKind::TYPE_FLOAT:
                    return llvm::ConstantFP::get(FloatTy, 0.0);
                case ASTFloatingPointTypeKind::TYPE_DOUBLE:
                    return llvm::ConstantFP::get(DoubleTy, 0.0);
            }
        }

        case ASTTypeKind::TYPE_ARRAY:
            return ConstantAggregateZero::get(Ty);

        case ASTTypeKind::TYPE_IDENTITY:
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

        // Bool
        case ASTTypeKind::TYPE_BOOL:
            return llvm::ConstantInt::get(BoolTy, ((ASTBoolValue *)Val)->getValue(), false);

        // Integer
        case ASTTypeKind::TYPE_INTEGER: {
            ASTIntegerType *IntegerType = (ASTIntegerType *) Type;
            switch (IntegerType->getIntegerKind()) {
                case ASTIntegerTypeKind::TYPE_BYTE:
                    return llvm::ConstantInt::get(Int8Ty, ((ASTIntegerValue *) Val)->getValue(), false);
                case ASTIntegerTypeKind::TYPE_USHORT:
                    return llvm::ConstantInt::get(Int16Ty, ((ASTIntegerValue *) Val)->getValue(), false);
                case ASTIntegerTypeKind::TYPE_SHORT:
                    return llvm::ConstantInt::get(Int16Ty, ((ASTIntegerValue *) Val)->getValue(), true);
                case ASTIntegerTypeKind::TYPE_UINT:
                    return llvm::ConstantInt::get(Int32Ty, ((ASTIntegerValue *) Val)->getValue(), false);
                case ASTIntegerTypeKind::TYPE_INT:
                    return llvm::ConstantInt::get(Int32Ty, ((ASTIntegerValue *) Val)->getValue(), true);
                case ASTIntegerTypeKind::TYPE_ULONG:
                    return llvm::ConstantInt::get(Int64Ty, ((ASTIntegerValue *) Val)->getValue(), false);
                case ASTIntegerTypeKind::TYPE_LONG:
                    return llvm::ConstantInt::get(Int64Ty, ((ASTIntegerValue *) Val)->getValue(), true);
            }
        }

        // Floating Point
        case ASTTypeKind::TYPE_FLOATING_POINT: {
            ASTFloatingPointType *FloatingPointType = (ASTFloatingPointType *) Type;
            switch (FloatingPointType->getFloatingPointKind()) {
                case ASTFloatingPointTypeKind::TYPE_FLOAT:
                    return llvm::ConstantFP::get(FloatTy, ((ASTFloatingValue *) Val)->getValue());
                case ASTFloatingPointTypeKind::TYPE_DOUBLE:
                    return llvm::ConstantFP::get(DoubleTy, ((ASTFloatingValue *) Val)->getValue());
            }
        }

        // Array
        case ASTTypeKind::TYPE_ARRAY: {
            llvm::ArrayType *ArrType = GenArrayType((ASTArrayType *) Type);
            std::vector<llvm::Constant *> Values;
            for (ASTValue *Value : ((ASTArrayValue *) Val)->getValues()) {
                llvm::Constant * V = GenValue(((ASTArrayType *) Type)->getType(), Value);
                Values.push_back(V);
            }
            return llvm::ConstantArray::get(ArrType, makeArrayRef(Values));
        }

        case ASTTypeKind::TYPE_STRING: {
            return Builder->CreateGlobalStringPtr(((ASTStringValue *) Val)->getValue());
        }

        // Identity
        case ASTTypeKind::TYPE_IDENTITY:
            break;

        // Void
        case ASTTypeKind::TYPE_VOID:
            // FIXME
            break;
    }
    assert(0 && "Unknown Type");
}

void CodeGenModule::GenStmt(CodeGenFunctionBase *CGF, ASTStmt * Stmt) {
    FLY_DEBUG("CodeGenModule", "GenStmt");
    switch (Stmt->getKind()) {

        // Var Assignment
        case ASTStmtKind::STMT_VAR: {
            ASTVarStmt *VarStmt = (ASTVarStmt *) Stmt;

            ASTVarRef *VarRef = VarStmt->getVarRef();

            if (VarStmt->getExpr()) {
                llvm::Value *V = GenExpr(VarStmt->getExpr());
                if (VarRef->getDef()->getType()->isIdentity() &&
                    ((ASTIdentityType *) VarStmt->getVarRef()->getDef()->getType())->isClass()) {

                    // set Value from CodeGen Instance
                    ((CodeGenInstance *) VarRef->getDef()->getCodeGen())->Init(V);

                }

                if (VarRef->getParent()) {
                    if (VarRef->getParent()->isCall()) { // TODO iterative parents
                        // TODO
                    } else if (VarRef->getParent()->isVarRef()) {
                        CodeGenInstance *CGI = ((CodeGenInstance *) ((ASTVarRef *) VarRef->getParent())->getDef()->getCodeGen());
                        CGI->getVar(VarRef->getDef()->getName())->Store(V);
                    }
                } else {
                    VarRef->getDef()->getCodeGen()->Store(V);
                }
            }
            break;
        }

        // Stmt with Expr
        case ASTStmtKind::STMT_EXPR: {
            ASTExprStmt *ExprStmt = (ASTExprStmt *) Stmt;
            GenExpr(ExprStmt->getExpr());
            break;
        }

        // Block of Stmt
        case ASTStmtKind::STMT_BLOCK: {
            ASTBlockStmt *Block = (ASTBlockStmt *) Stmt;
            switch (Block->getKind()) {
                case ASTStmtKind::STMT_BLOCK:
                    GenBlock(CGF, Block->getContent());
                    break;
                case ASTStmtKind::STMT_IF:
                    GenIfBlock(CGF, (ASTIfStmt *)Block);
                    break;
                case ASTStmtKind::STMT_SWITCH:
                    GenSwitchBlock(CGF, (ASTSwitchStmt *)Block);
                    break;
                case ASTStmtKind::STMT_LOOP: {
                    ASTLoopStmt *LoopBlock = (ASTLoopStmt *) Block;
                    if (!LoopBlock->getPost()->isEmpty()) {
                        GenForBlock(CGF, LoopBlock);
                    } else {
                        GenWhileBlock(CGF, LoopBlock);
                    }
                    break;
                }
                case ASTStmtKind::STMT_LOOP_IN: {

                }
            }
            break;
        }

        // Delete Stmt
        case ASTStmtKind::STMT_DELETE: {
            ASTDeleteStmt *Delete = (ASTDeleteStmt *) Stmt;
            ASTVar * Var = Delete->getVarRef()->getDef();
            if (Var->getType()->getKind() == ASTTypeKind::TYPE_IDENTITY) {
                if (!((ASTClass *) ((ASTClassType *) Var->getType())->getDef())->getCodeGen()->getVars().empty()) {
                    Instruction *I = CallInst::CreateFree(Var->getCodeGen()->getPointer(), Builder->GetInsertBlock());
                    Builder->Insert(I);
                }
            }
            break;
        }

        // Break Stmt
        case ASTStmtKind::STMT_BREAK:
            // TODO go to break BB
            break;

        // Continue Stmt
        case ASTStmtKind::STMT_CONTINUE:
            // TODO go to continue BB
            break;

        // Return Stmt
        case ASTStmtKind::STMT_RETURN: {
            ASTReturnStmt *Return = (ASTReturnStmt *) Stmt;
            if (Return->getParent()->getKind() == ASTStmtKind::STMT_BLOCK) {
                llvm::Value *V = Return->getExpr() ? GenExpr(Return->getExpr()) : nullptr;
                GenReturn(((ASTBlockStmt *) Return->getParent())->getTop(), V);
            }
            break;
        }

        case ASTStmtKind::STMT_HANDLE: {
            ASTHandleStmt *HandleStmt = (ASTHandleStmt *) Stmt;

            CodeGenHandle *CGH = new CodeGenHandle(this);
            HandleStmt->setCodeGen(CGH);
            llvm::BasicBlock *HandleBB = CGH->GenBlock();
            Builder->CreateBr(HandleBB);
            Builder->SetInsertPoint(HandleBB);
            GenStmt(CGF, HandleStmt);
            break;
        }
        
        case ASTStmtKind::STMT_FAIL: {
            ASTFailStmt *FailStmt = (ASTFailStmt *) Stmt;

            // Take the current ErrorHandler CodeGen
            ASTVar *ErrorHandler = FailStmt->getErrorHandler();
            CodeGenError *CGE = (CodeGenError *) ErrorHandler->getCodeGen();

            // Store Fail value in ErrorHandler
            llvm::Value *V = GenExpr(FailStmt->getExpr());
            CGE->Store(V);

            // Generate Return with default value for stop execution flow
            if (FailStmt->hasHandle()) {
                ASTHandleStmt *HandleStmt = FailStmt->getHandle();
                HandleStmt->getCodeGen()->GoToBlock();
            } else {
                ASTFunctionBase *F = FailStmt->getParent()->getTop();
                GenReturn(F);
            }
            break;
        }
    }
}

CodeGenError *CodeGenModule::GenErrorHandler(ASTVar *Var) {
    if (!Var->getType()->isError()) {
        // Error:
    }
    // Set CodeGenError
    return new CodeGenError(this, Var);
}

CodeGenVarBase *CodeGenModule::GenVar(ASTVar *Var) {
    if (Var->getType()->isIdentity() && ((ASTIdentityType *) Var->getType())->isClass()) {
        return new CodeGenInstance(this, Var);
    }
    return new CodeGenVar(this, Var);
}

llvm::Value *CodeGenModule::GenVarRef(ASTVarRef *VarRef) {
    if (VarRef->getDef() == nullptr) {
        Diag(VarRef->getLocation(), diag::err_unref_var);
        return nullptr;
    }

    // Class Var
    if (VarRef->getDef()->getVarKind() == ASTVarKind::VAR_CLASS) {

        // Return the instance value
        if (VarRef->getParent() && VarRef->getParent()->isCall()) { // TODO iterative parents
            // TODO
        } else if (VarRef->getParent() && VarRef->getParent()->isVarRef()) {
            // get Value from CodeGen Instance (set in GenStmt())
            CodeGenInstance *CGI = (CodeGenInstance *) ((ASTVarRef *) VarRef->getParent())->getDef()->getCodeGen();
            CodeGenClassVar *CGV = CGI->getVar(VarRef->getDef()->getName());
            return CGV->getValue();
        } else { // Return static value
            return VarRef->getDef()->getCodeGen()->getValue();
        }
    }

    // Local Var
    // Return the Value
    return VarRef->getDef()->getCodeGen()->getValue();
}

llvm::Value *CodeGenModule::GenCall(ASTCall *Call) {
    FLY_DEBUG_MESSAGE("CodeGenModule", "GenCall",
                      "Call=" << Call->str());
    // Check if Func is declared
    if (Call->getDef() == nullptr) {
        Diag(Call->getLocation(), diag::err_unref_call);
        return nullptr;
    }

    // The function arguments
    llvm::SmallVector<llvm::Value *, 8> Args;
    // Add error as first param
    CodeGenError *CGE = (CodeGenError *) Call->getDef()->getErrorHandler()->getCodeGen();
    Args.push_back(CGE->getPointer()); // Error is a Pointer

    // Take the CGI Value and pass to Call as first argument
    llvm::Value *Instance = nullptr;

    // Check Function
    if (Call->getDef()->getKind() == ASTFunctionKind::FUNCTION) {
        ASTFunction *Def = (ASTFunction *) Call->getDef(); // is resolved
    }
    else if (Call->getDef()->getKind() == ASTFunctionKind::CLASS_METHOD) {
        ASTClassMethod *Def = (ASTClassMethod *) Call->getDef();

        if (Def->isConstructor()) { // Call class constructor
            llvm::StructType *Ty = Def->getClass()->getCodeGen()->getType();
            Constant* AllocSize = ConstantExpr::getSizeOf(Ty);
            if (Def->getClass()->getVars().empty()) { // size is zero
                Instance = Builder->CreateAlloca(Def->getClass()->getCodeGen()->getType()); // alloca into the Stack
            } else { // size is greater than zero
                AllocSize = ConstantExpr::getTruncOrBitCast(AllocSize, Int32Ty);
                // @malloc data type struct
                Instruction *I = CallInst::CreateMalloc(Builder->GetInsertBlock(), Int32Ty, Ty, AllocSize, nullptr, nullptr);
                Instance = Builder->Insert(I, ((ASTClassMethod *) Call->getDef())->getName() + "Inst");
            }
            Args.push_back(Instance);
        } else if (Call->getParent() && Call->getParent()->isCall()) { // TODO iterative parents
            // TODO
        } else if (Call->getParent() && Call->getParent()->isVarRef()) {
            Args.push_back(((ASTVarRef *) Call->getParent())->getDef()->getCodeGen()->getPointer());
        }
        // else { // call static method }
    }

    // Return Call Value
    pushArgs(Call, Args);
    llvm::Value *RetVal = Builder->CreateCall(Call->getDef()->getCodeGen()->getFunction(), Args);

    return Instance == nullptr ? RetVal : Instance;
}

llvm::Value *CodeGenModule::GenExpr(ASTExpr *Expr) {
    FLY_DEBUG("CodeGenModule", "GenExpr");
    CodeGenExpr *CGExpr = new CodeGenExpr(this, Expr);
    return CGExpr->getValue();
}

void CodeGenModule::GenBlock(CodeGenFunctionBase *CGF, const llvm::SmallVector<ASTStmt *, 8> &Content, llvm::BasicBlock *BB) {
    FLY_DEBUG("CodeGenModule", "GenBlock");
    if (BB) Builder->SetInsertPoint(BB);
    for (ASTStmt *Stmt : Content) {
        GenStmt(CGF, Stmt);
    }
}

void CodeGenModule::GenIfBlock(CodeGenFunctionBase *CGF, ASTIfStmt *If) {
    FLY_DEBUG("CodeGenModule", "GenIfBlock");
    llvm::Function *Fn = CGF->getFunction();

    // If Block
    llvm::Value *IfCond = GenExpr(If->getCondition());
    llvm::BasicBlock *IfBB = llvm::BasicBlock::Create(LLVMCtx, "ifthen", Fn);

    // Create End block
    llvm::BasicBlock *EndBB = llvm::BasicBlock::Create(LLVMCtx, "endif", Fn);

    if (!If->getElse()) {

        if (If->getElsif().empty()) { // If ...
            Builder->CreateCondBr(IfCond, IfBB, EndBB);
            GenBlock(CGF, If->getBlock()->getContent(), IfBB);
            Builder->CreateBr(EndBB);
        } else { // If - elsif ...
            llvm::BasicBlock *ElsifBB = llvm::BasicBlock::Create(LLVMCtx, "elsif", Fn, EndBB);
            Builder->CreateCondBr(IfCond, IfBB, ElsifBB);

            // Start if-then
            GenBlock(CGF, If->getBlock()->getContent(), IfBB);
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
                ASTElsif *Elsif = If->getElsif()[i];
                Builder->SetInsertPoint(ElsifBB);
                llvm::Value *ElsifCond = GenExpr(Elsif->getCondition());
                Builder->CreateCondBr(ElsifCond, ElsifThenBB, NextElsifBB);

                GenBlock(CGF, Elsif->getBlock()->getContent(), ElsifThenBB);
                Builder->CreateBr(EndBB);

                ElsifBB = NextElsifBB;
            }
        }

    } else {

        // Create Else block
        llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(LLVMCtx, "else", Fn, EndBB);

        if (If->getElsif().empty()) { // If - Else
            Builder->CreateCondBr(IfCond, IfBB, ElseBB);
            GenBlock(CGF, If->getBlock()->getContent(), IfBB);
            Builder->CreateBr(EndBB);
        } else { // If - Elsif - Else
            llvm::BasicBlock *ElsifBB = llvm::BasicBlock::Create(LLVMCtx, "elsif", Fn, ElseBB);
            Builder->CreateCondBr(IfCond, IfBB, ElsifBB);

            // Start if-then
            GenBlock(CGF, If->getBlock()->getContent(), IfBB);
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
                ASTElsif *Elsif = If->getElsif()[i];
                Builder->SetInsertPoint(ElsifBB);
                llvm::Value *ElsifCond = GenExpr(Elsif->getCondition());
                Builder->CreateCondBr(ElsifCond, ElsifThenBB, NextElsifBB);

                GenBlock(CGF, Elsif->getBlock()->getContent(), ElsifThenBB);
                Builder->CreateBr(EndBB);

                ElsifBB = NextElsifBB;
            }
        }

        GenBlock(CGF, If->getElse()->getContent(), ElseBB);
        Builder->CreateBr(EndBB);
    }

    // Continue insertions into End Branch
    Builder->SetInsertPoint(EndBB);
}

llvm::BasicBlock *CodeGenModule::GenElsifBlock(CodeGenFunctionBase *CGF,
                                               llvm::BasicBlock *ElsifBB,
                                               llvm::SmallVector<ASTElsif *, 8>::iterator &It) {
    FLY_DEBUG("CodeGenModule", "GenElsifBlock");
    llvm::Function *Fn = CGF->getFunction();
    ASTElsif *&Elsif = *It;
    It++;
    if (*It == nullptr) {
        return ElsifBB;
    } else {
        Builder->SetInsertPoint(ElsifBB);
        llvm::Value *Cond = GenExpr(Elsif->getCondition());
        llvm::BasicBlock *NextElsifBB = llvm::BasicBlock::Create(LLVMCtx, "elsif", Fn);
        Builder->CreateCondBr(Cond, ElsifBB, NextElsifBB);

        llvm::BasicBlock *ElsifThenBB = llvm::BasicBlock::Create(LLVMCtx, "elsifthen", Fn);
        GenBlock(CGF, Elsif->getBlock()->getContent(), ElsifThenBB);
        return GenElsifBlock(CGF, ElsifThenBB, It);
    }
}

void CodeGenModule::GenSwitchBlock(CodeGenFunctionBase *CGF, ASTSwitchStmt *Switch) {
    FLY_DEBUG("CodeGenModule", "GenSwitchBlock");
    llvm::Function *Fn = CGF->getFunction();

    // Create End Block
    llvm::BasicBlock *EndBR = llvm::BasicBlock::Create(LLVMCtx, "endswitch", Fn);

    // Create Expression evaluator for Switch
    llvm::Value *SwitchVal = Switch->getVarRef()->getDef()->getCodeGen()->getValue();
    llvm::SwitchInst *Inst = Builder->CreateSwitch(SwitchVal, EndBR);

    // Create Cases
    unsigned long Size = Switch->getCases().size();

    llvm::BasicBlock *NextCaseBB = nullptr;
    for (int i=0; i < Size; i++) {
        ASTSwitchCase *Case = Switch->getCases()[i];
        llvm::Value *CaseVal = GenValue(Case->getType(), Case->getValue());
        llvm::ConstantInt *CaseConst = llvm::cast<llvm::ConstantInt, llvm::Value>(CaseVal);
        llvm::BasicBlock *CaseBB = NextCaseBB == nullptr ?
                                   llvm::BasicBlock::Create(LLVMCtx, "case", Fn, EndBR) : NextCaseBB;
        Inst->addCase(CaseConst, CaseBB);
        GenBlock(CGF, Case->getBlock()->getContent(), CaseBB);

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
        GenBlock(CGF, Switch->getDefault()->getContent(), DefaultBB);
        Builder->CreateBr(EndBR);
    }

    // Continue insertions into End Branch
    Builder->SetInsertPoint(EndBR);
}

void CodeGenModule::GenForBlock(CodeGenFunctionBase *CGF, ASTLoopStmt *Loop) {
    FLY_DEBUG("CodeGenModule", "GenForBlock");
    llvm::Function *Fn = CGF->getFunction();

    // Create Loop Block
    llvm::BasicBlock *LoopBB = LoopBB = llvm::BasicBlock::Create(LLVMCtx, "forloop", Fn);

    // Create Post Block
    llvm::BasicBlock *PostBB = nullptr;
    if (Loop->getPost() && !Loop->getPost()->isEmpty()) {
        PostBB = llvm::BasicBlock::Create(LLVMCtx, "forpost", Fn);
    }

    // Create End Block
    llvm::BasicBlock *EndBB = llvm::BasicBlock::Create(LLVMCtx, "endfor", Fn);

    // Add to Condition
    llvm::BasicBlock *CondBB = nullptr;
    if (Loop->getCondition()) {
        CondBB = llvm::BasicBlock::Create(LLVMCtx, "forcond", Fn, LoopBB);
        Builder->CreateBr(CondBB);

        // Create Condition
        Builder->SetInsertPoint(CondBB);
        Value *Cond = GenExpr(Loop->getCondition());
        Builder->CreateCondBr(Cond, LoopBB, EndBB);
    } else {
        Builder->CreateBr(LoopBB);
    }

    // Add to Loop
    GenBlock(CGF, Loop->getBlock()->getContent(), LoopBB);
    if (PostBB) {
        Builder->CreateBr(PostBB);
    } else if (CondBB) {
        Builder->CreateBr(CondBB);
    } else {
        Builder->CreateBr(LoopBB);
    }

    // Add to Post
    if (PostBB) {
        GenBlock(CGF, Loop->getPost()->getContent(), PostBB);
        if (CondBB) {
            Builder->CreateBr(CondBB);
        } else {
            Builder->CreateBr(LoopBB);
        }
    }

    // Continue insertions into End Branch
    Builder->SetInsertPoint(EndBB);
}

void CodeGenModule::GenWhileBlock(CodeGenFunctionBase *CGF, ASTLoopStmt *While) {
    FLY_DEBUG("CodeGenModule", "GenWhileBlock");
    llvm::Function *Fn = CGF->getFunction();

    // Create Expression evaluator for While
    llvm::BasicBlock *CondBR = llvm::BasicBlock::Create(LLVMCtx, "whilecond", Fn);

    // Create Loop Block
    llvm::BasicBlock *LoopBR = llvm::BasicBlock::Create(LLVMCtx, "whileloop", Fn);

    // Create End Block
    llvm::BasicBlock *EndBR = llvm::BasicBlock::Create(LLVMCtx, "whileend", Fn);

    Builder->CreateBr(CondBR); // goto Condition Branch

    // Condition Branch
    Builder->SetInsertPoint(CondBR);
    llvm::Value *Cond = GenExpr(While->getCondition());
    Builder->CreateCondBr(Cond, LoopBR, EndBR); // iF condition is true goto Loop Branch else goto End Branch

    // The While Block
    GenBlock(CGF, While->getBlock()->getContent(), LoopBR);
    Builder->CreateBr(CondBR);

    // Continue insertions into End Branch
    Builder->SetInsertPoint(EndBR);
}

void CodeGenModule::pushArgs(ASTCall *Call, llvm::SmallVector<llvm::Value *, 8> &Args) {
    // Add Call arguments to Function args
    for (ASTArg *Arg : Call->getArgs()) {
        llvm::Value *V = GenExpr(Arg->getExpr());
        Args.push_back(V);
    }
}

void CodeGenModule::GenReturn(ASTFunctionBase *F, llvm::Value *V) {
    // Create the Value for return
    if (F->getType()->isVoid()) {
        Builder->CreateRetVoid();
    } else {
        Value *Ret = V ? V : GenDefaultValue(F->getType());
        Builder->CreateRet(Ret);
    }
}
