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
#include "CodeGen/CodeGenFail.h"
#include "Sema/SemaBuilder.h"
#include "AST/ASTImport.h"
#include "AST/ASTNode.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTDeleteStmt.h"
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
#include "AST/ASTVarStmt.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTClass.h"
#include "AST/ASTEnum.h"
#include "AST/ASTError.h"
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

    // Create Error Struct Type
    ErrorType = CodeGenFail::GenErrorType(this);
    
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
            ASTIdentityTypeKind IdentityTypeKind = ((ASTIdentityType *) Type)->getIdentityKind();

            // Error: unreferenced
            if (IdentityTypeKind == ASTIdentityTypeKind::TYPE_NONE) {
                Diag(Type->getLocation(), diag::err_cg_unref_identity_type) << ((ASTIdentityType *) Type)->getName();
                return nullptr;
            }

            ASTIdentityType *a = ((ASTIdentityType *) Type)->getDef()->getType();
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
            ASTVarStmt *VarDefine = (ASTVarStmt *) Stmt;

            ASTVarRef *VarRef = VarDefine->getVarRef();

            if (VarDefine->getExpr()) {
                llvm::Value *V = GenExpr(CGF, VarRef->getDef()->getType(), VarDefine->getExpr());
                if (VarRef->getDef()->getType()->isIdentity() &&
                    ((ASTIdentityType *) VarDefine->getVarRef()->getDef()->getType())->isClass()) {

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
            GenExpr(CGF, ExprStmt->getExpr());
            break;
        }

        // Block of Stmt
        case ASTStmtKind::STMT_BLOCK: {
            ASTBlock *Block = (ASTBlock *) Stmt;
            switch (Block->getBlockKind()) {
                case ASTBlockKind::BLOCK:
                    GenBlock(CGF, Block->getContent());
                    break;
                case ASTBlockKind::BLOCK_IF:
                    GenIfBlock(CGF, (ASTIfBlock *)Block);
                    break;
                case ASTBlockKind::BLOCK_ELSIF:
                case ASTBlockKind::BLOCK_ELSE:
                    // All done into BLOCK_STMT_IF
                    break;
                case ASTBlockKind::BLOCK_SWITCH:
                    GenSwitchBlock(CGF, (ASTSwitchBlock *)Block);
                    break;
                case ASTBlockKind::BLOCK_SWITCH_CASE:
                case ASTBlockKind::BLOCK_SWITCH_DEFAULT:
                    // All done into BLOCK_STMT_SWITCH
                    break;
                case ASTBlockKind::BLOCK_FOR:
                    GenForBlock(CGF, (ASTForBlock *)Block);
                    break;
                case ASTBlockKind::BLOCK_WHILE:
                    GenWhileBlock(CGF, (ASTWhileBlock *)Block);
                    break;
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
            // TODO
            break;

        // Continue Stmt
        case ASTStmtKind::STMT_CONTINUE:
            // TODO
            break;

        // Return Stmt
        case ASTStmtKind::STMT_RETURN: {
            ASTReturnStmt *Return = (ASTReturnStmt *) Stmt;
            if (Return->getParent()->getKind() == ASTStmtKind::STMT_BLOCK) {
                llvm::Value *V = Return->getExpr() ? GenExpr(CGF, ((ASTBlock *) Return->getParent())->getTop()->getType(),
                                         Return->getExpr()) : nullptr;
                GenReturn(((ASTBlock *) Return->getParent())->getTop(), V);
            }
            break;
        }
    }
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

llvm::Value *CodeGenModule::GenCall(CodeGenFunctionBase *CGF, ASTCall *Call) {
    FLY_DEBUG_MESSAGE("CodeGenModule", "GenCall",
                      "Call=" << Call->str());
    // Check if Func is declared
    if (Call->getDef() == nullptr) {
        Diag(Call->getLocation(), diag::err_unref_call);
        return nullptr;
    }

    // The function arguments
    llvm::SmallVector<llvm::Value *, 8> Args;

    // Take the CGI Value and pass to Call as first argument
    llvm::Value *Instance = nullptr;
    if (Call->getDef()->getKind() == ASTFunctionKind::FUNCTION) {
        ASTFunction *Def = (ASTFunction *) Call->getDef(); // is resolved
        if (Def->getName() == "fail") {
            CodeGenFail::GenSTMT(CGF, Call);
            return nullptr;
        }

        // Add error as first param
        Args.push_back(CGF->getErrorVar());
    }
    else if (Call->getDef()->getKind() == ASTFunctionKind::CLASS_FUNCTION) {
        ASTClassFunction *Def = (ASTClassFunction *) Call->getDef();

        if (Def->getClass()->getClassKind() != ASTClassKind::STRUCT) { // not for struct
            // Add error as first param
            Args.push_back(CGF->getErrorVar());
        }
        if (Def->isConstructor()) { // Call class constructor
            llvm::StructType *Ty = Def->getClass()->getCodeGen()->getType();
            Constant* AllocSize = ConstantExpr::getSizeOf(Ty);
            if (Def->getClass()->getVars().empty()) { // size is zero
                Instance = Builder->CreateAlloca(Def->getClass()->getCodeGen()->getType()); // alloca into the Stack
            } else { // size is greater than zero
                AllocSize = ConstantExpr::getTruncOrBitCast(AllocSize, Int32Ty);
                // @malloc data type struct
                Instruction *I = CallInst::CreateMalloc(Builder->GetInsertBlock(), Int32Ty, Ty, AllocSize, nullptr, nullptr);
                Instance = Builder->Insert(I, Call->getDef()->getName() + "Inst");
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
    pushArgs(CGF, Call, Args);
    llvm::Value *RetVal = Builder->CreateCall(Call->getDef()->getCodeGen()->getFunction(), Args);

    return Instance == nullptr ? RetVal : Instance;
}

llvm::Value *CodeGenModule::GenExpr(CodeGenFunctionBase *CGF, ASTExpr *Expr) {
    FLY_DEBUG("CodeGenModule", "GenExpr");
    CodeGenExpr *CGExpr = new CodeGenExpr(this, CGF, Expr);
    return CGExpr->getValue();
}

// FIXME to remove ASTType parameter the expr need to have the type specified
llvm::Value *CodeGenModule::GenExpr(CodeGenFunctionBase *CGF, const ASTType *Type, ASTExpr *Expr) {
    FLY_DEBUG("CodeGenModule", "GenExpr");
    CodeGenExpr *CGExpr = new CodeGenExpr(this, CGF, Expr, Type);
    return CGExpr->getValue();
}

void CodeGenModule::GenBlock(CodeGenFunctionBase *CGF, const std::vector<ASTStmt *> &Content, llvm::BasicBlock *BB) {
    FLY_DEBUG("CodeGenModule", "GenBlock");
    if (BB) Builder->SetInsertPoint(BB);
    for (ASTStmt *Stmt : Content) {
        GenStmt(CGF, Stmt);
    }
}

void CodeGenModule::GenIfBlock(CodeGenFunctionBase *CGF, ASTIfBlock *If) {
    FLY_DEBUG("CodeGenModule", "GenIfBlock");
    llvm::Function *Fn = CGF->getFunction();
    ASTBoolType * BoolType = SemaBuilder::CreateBoolType(SourceLocation()); // used to force bool in condition expr

    // If Block
    llvm::Value *IfCond = GenExpr(CGF, BoolType, If->getCondition());
    llvm::BasicBlock *IfBB = llvm::BasicBlock::Create(LLVMCtx, "ifthen", Fn);

    // Create End block
    llvm::BasicBlock *EndBB = llvm::BasicBlock::Create(LLVMCtx, "endif", Fn);

    if (!If->getElseBlock()) {

        if (If->getElsifBlocks().empty()) { // If ...
            Builder->CreateCondBr(IfCond, IfBB, EndBB);
            GenBlock(CGF, If->getContent(), IfBB);
            Builder->CreateBr(EndBB);
        } else { // If - elsif ...
            llvm::BasicBlock *ElsifBB = llvm::BasicBlock::Create(LLVMCtx, "elsif", Fn, EndBB);
            Builder->CreateCondBr(IfCond, IfBB, ElsifBB);

            // Start if-then
            GenBlock(CGF, If->getContent(), IfBB);
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
                llvm::Value *ElsifCond = GenExpr(CGF, BoolType, Elsif->getCondition());
                Builder->CreateCondBr(ElsifCond, ElsifThenBB, NextElsifBB);

                GenBlock(CGF, Elsif->getContent(), ElsifThenBB);
                Builder->CreateBr(EndBB);

                ElsifBB = NextElsifBB;
            }
        }

    } else {

        // Create Else block
        llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(LLVMCtx, "else", Fn, EndBB);

        if (If->getElsifBlocks().empty()) { // If - Else
            Builder->CreateCondBr(IfCond, IfBB, ElseBB);
            GenBlock(CGF, If->getContent(), IfBB);
            Builder->CreateBr(EndBB);
        } else { // If - Elsif - Else
            llvm::BasicBlock *ElsifBB = llvm::BasicBlock::Create(LLVMCtx, "elsif", Fn, ElseBB);
            Builder->CreateCondBr(IfCond, IfBB, ElsifBB);

            // Start if-then
            GenBlock(CGF, If->getContent(), IfBB);
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
                llvm::Value *ElsifCond = GenExpr(CGF, BoolType, Elsif->getCondition());
                Builder->CreateCondBr(ElsifCond, ElsifThenBB, NextElsifBB);

                GenBlock(CGF, Elsif->getContent(), ElsifThenBB);
                Builder->CreateBr(EndBB);

                ElsifBB = NextElsifBB;
            }
        }

        GenBlock(CGF, If->getElseBlock()->getContent(), ElseBB);
        Builder->CreateBr(EndBB);
    }

    // Continue insertions into End Branch
    Builder->SetInsertPoint(EndBB);
}

llvm::BasicBlock *CodeGenModule::GenElsifBlock(CodeGenFunctionBase *CGF,
                                               llvm::BasicBlock *ElsifBB,
                                               std::vector<ASTElsifBlock *>::iterator &It) {
    FLY_DEBUG("CodeGenModule", "GenElsifBlock");
    llvm::Function *Fn = CGF->getFunction();
    ASTElsifBlock *&Elsif = *It;
    It++;
    if (*It == nullptr) {
        return ElsifBB;
    } else {
        Builder->SetInsertPoint(ElsifBB);
        ASTBoolType * BoolType = SemaBuilder::CreateBoolType(SourceLocation());
        llvm::Value *Cond = GenExpr(CGF, BoolType, Elsif->getCondition());
        llvm::BasicBlock *NextElsifBB = llvm::BasicBlock::Create(LLVMCtx, "elsif", Fn);
        Builder->CreateCondBr(Cond, ElsifBB, NextElsifBB);

        llvm::BasicBlock *ElsifThenBB = llvm::BasicBlock::Create(LLVMCtx, "elsifthen", Fn);
        GenBlock(CGF, Elsif->getContent(), ElsifThenBB);
        return GenElsifBlock(CGF, ElsifThenBB, It);
    }
}

void CodeGenModule::GenSwitchBlock(CodeGenFunctionBase *CGF, ASTSwitchBlock *Switch) {
    FLY_DEBUG("CodeGenModule", "GenSwitchBlock");
    llvm::Function *Fn = CGF->getFunction();
    ASTIntType * IntType = SemaBuilder::CreateIntType(SourceLocation()); // used to force int in switch case expr valuation

    // Create End Block
    llvm::BasicBlock *EndBR = llvm::BasicBlock::Create(LLVMCtx, "endswitch", Fn);

    // Create Expression evaluator for Switch
    llvm::Value *SwitchVal = GenExpr(CGF, IntType, Switch->getExpr());
    llvm::SwitchInst *Inst = Builder->CreateSwitch(SwitchVal, EndBR);

    // Create Cases
    unsigned long Size = Switch->getCases().size();

    llvm::BasicBlock *NextCaseBB = nullptr;
    for (int i=0; i < Size; i++) {
        ASTSwitchCaseBlock *Case = Switch->getCases()[i];
        llvm::Value *CaseVal = GenExpr(CGF, IntType, Case->getExpr());
        llvm::ConstantInt *CaseConst = llvm::cast<llvm::ConstantInt, llvm::Value>(CaseVal);
        llvm::BasicBlock *CaseBB = NextCaseBB == nullptr ?
                                   llvm::BasicBlock::Create(LLVMCtx, "case", Fn, EndBR) : NextCaseBB;
        Inst->addCase(CaseConst, CaseBB);
        GenBlock(CGF, Case->getContent(), CaseBB);

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

void CodeGenModule::GenForBlock(CodeGenFunctionBase *CGF, ASTForBlock *For) {
    FLY_DEBUG("CodeGenModule", "GenForBlock");
    llvm::Function *Fn = CGF->getFunction();
    ASTBoolType * BoolType = SemaBuilder::CreateBoolType(SourceLocation()); // used to force bool in condition expr

    // Add to Current Block
    if (!For->isEmpty()) {
        GenBlock(CGF, For->getContent());
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
        Value *Cond = GenExpr(CGF, BoolType, For->getCondition());
        Builder->CreateCondBr(Cond, LoopBB, EndBB);
    } else {
        Builder->CreateBr(LoopBB);
    }

    // Add to Loop
    GenBlock(CGF, For->getLoop()->getContent(), LoopBB);
    if (PostBB) {
        Builder->CreateBr(PostBB);
    } else if (CondBB) {
        Builder->CreateBr(CondBB);
    } else {
        Builder->CreateBr(LoopBB);
    }

    // Add to Post
    if (PostBB) {
        GenBlock(CGF, For->getPost()->getContent(), PostBB);
        if (CondBB) {
            Builder->CreateBr(CondBB);
        } else {
            Builder->CreateBr(LoopBB);
        }
    }

    // Continue insertions into End Branch
    Builder->SetInsertPoint(EndBB);
}

void CodeGenModule::GenWhileBlock(CodeGenFunctionBase *CGF, ASTWhileBlock *While) {
    FLY_DEBUG("CodeGenModule", "GenWhileBlock");
    llvm::Function *Fn = CGF->getFunction();
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
    llvm::Value *Cond = GenExpr(CGF, BoolType, While->getCondition());
    Builder->CreateCondBr(Cond, LoopBR, EndBR); // iF condition is true goto Loop Branch else goto End Branch

    // The While Block
    GenBlock(CGF, While->getContent(), LoopBR);
    Builder->CreateBr(CondBR);

    // Continue insertions into End Branch
    Builder->SetInsertPoint(EndBR);
}

void CodeGenModule::pushArgs(CodeGenFunctionBase *CGF, ASTCall *Call, llvm::SmallVector<llvm::Value *, 8> &Args) {
    // Add Call arguments to Function args
    for (ASTArg *Arg : Call->getArgs()) {
        llvm::Value *V = GenExpr(CGF, Arg->getDef()->getType(), Arg->getExpr());
        Args.push_back(V);
    }
}

void CodeGenModule::GenReturn(ASTFunctionBase *F, llvm::Value *V) {
    // Create the Value for return
    if (F->getType()->isVoid()) {
        Builder->CreateRetVoid();
    } else {
        Builder->CreateRet(V);
    }
}
