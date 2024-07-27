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
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenFunction.h"
#include "CodeGen/CodeGenClass.h"
#include "CodeGen/CodeGenEnumEntry.h"
#include "CodeGen/CodeGenGlobalVar.h"
#include "CodeGen/CodeGenVar.h"
#include "CodeGen/CodeGenExpr.h"
#include "CodeGen/CodeGenHandle.h"
#include "CodeGen/CodeGenError.h"
#include "AST/ASTImport.h"
#include "AST/ASTModule.h"
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
#include "AST/ASTEnumEntry.h"
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

CodeGenModule::CodeGenModule(DiagnosticsEngine &Diags, ASTModule &AST, LLVMContext &LLVMCtx,
                             TargetInfo &Target, CodeGenOptions &CGOpts) :
        Diags(Diags),
        AST(AST),
        Target(Target),
        Module(new llvm::Module(AST.getName(), LLVMCtx)),
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

    Int8PtrTy = Int8Ty->getPointerTo(0);
    Int8PtrPtrTy = Int8PtrTy->getPointerTo(0);

    PointerWidthInBits = Target.getPointerWidth(0);
    PointerAlignInBytes = toCharUnitsFromBits(Target.getPointerAlign(0)).getQuantity();
    SizeSizeInBytes = toCharUnitsFromBits(Target.getMaxPointerWidth()).getQuantity();
    IntAlignInBytes = toCharUnitsFromBits(Target.getIntAlign()).getQuantity();
    IntTy = llvm::IntegerType::get(LLVMCtx, Target.getIntWidth());
    IntPtrTy = llvm::IntegerType::get(LLVMCtx, Target.getMaxPointerWidth());
    AllocaInt8PtrTy = Int8Ty->getPointerTo(Module->getDataLayout().getAllocaAddrSpace());

    ErrorTy = CodeGenError::GenErrorType(LLVMCtx);
    ErrorPtrTy = llvm::PointerType::get(ErrorTy, 0);

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
}

DiagnosticBuilder CodeGenModule::Diag(const SourceLocation &Loc, unsigned DiagID) {
    return Diags.Report(Loc, DiagID);
}

llvm::Module *CodeGenModule::getModule() const {
    return Module;
}

ASTModule &CodeGenModule::getAst() const {
    return AST;
}

void CodeGenModule::GenHeaders() {
    GenAll();
}

void CodeGenModule::GenAll() {

    // Generate External GlobalVars
    for (const auto &Entry : AST.getExternalGlobalVars()) {
        ASTGlobalVar *GlobalVar = Entry.getValue();
        FLY_DEBUG_MESSAGE("FrontendAction", "GenerateCode",
                          "ExternalGlobalVar=" << GlobalVar->str());
        GenGlobalVar(GlobalVar, true);
    }

    // Generate External Function
    for (auto &StrMapEntry : AST.getExternalFunctions()) {
        for (auto &IntMap : StrMapEntry.getValue()) {
            for (auto &Function : IntMap.second) {
                FLY_DEBUG_MESSAGE("FrontendAction", "GenerateCode",
                                  "ExternalFunction=" << Function->str());
                GenFunction(Function, true);
            }
        }
    }

    // Generate GlobalVars
    std::vector<CodeGenGlobalVar *> CGGlobalVars;
    for (const auto &GlobalVar : AST.getGlobalVars()) {
        FLY_DEBUG_MESSAGE("FrontendAction", "GenerateCode",
                          "GlobalVar=" << GlobalVar->str());
        CodeGenGlobalVar *CGV = GenGlobalVar(GlobalVar);
        CGGlobalVars.push_back(CGV);
//        if (FrontendOpts.CreateHeader) {
//            CGH->AddGlobalVar(GlobalVar);
//        }
    }

    // Instantiates all Function CodeGen in order to be set in all Call references
    std::vector<CodeGenFunction *> CGFunctions;
    for (auto &FuncStrMap : AST.getFunctions()) {
        for (auto &FuncList : FuncStrMap.getValue()) {
            for (auto &Func : FuncList.second) {
                CodeGenFunction *CGF = GenFunction(Func);
                CGFunctions.push_back(CGF);
//                if (FrontendOpts.CreateHeader) {
//                    CGH->AddFunction(Func);
//                }
            }
        }
    }

    // Generate Identities: Class & Enum
    std::vector<CodeGenClass *> CGClasses;
    for (auto &Identity : AST.getIdentities()) {
        if (Identity->getTopDefKind() == ASTTopDefKind::DEF_CLASS) {
            CodeGenClass *CGC = GenClass((ASTClass *) Identity);
            CGClasses.push_back(CGC);
//                if (FrontendOpts.CreateHeader) {
//                    CGH->setClass((ASTClass *) Identity);
//                }
        } else if (Identity->getTopDefKind() == ASTTopDefKind::DEF_ENUM) {
            GenEnum((ASTEnum *) Identity);
        }
    }

    // Body must be generated after all CodeGen has been set for each TopDef
    for (auto CGF : CGFunctions) {
        FLY_DEBUG_MESSAGE("FrontendAction", "GenerateCode",
                          "FunctionBody=" << CGF->getName());
        CGF->GenBody();
    }

    // Generate Class Body
    for (auto CGClass : CGClasses) {
        for (auto CGCF: CGClass->getConstructors()) {
            CGCF->GenBody();
        }
        for (auto CGCF: CGClass->getFunctions()) {
            CGCF->GenBody();
        }
    }

}

/**
 * GenStmt from VarDecl
 * @param GlobalVar
 * @param isExternal
 */
CodeGenGlobalVar *CodeGenModule::GenGlobalVar(ASTGlobalVar* GlobalVar, bool isExternal) {
    FLY_DEBUG_MESSAGE("CodeGenModule", "GenGlobalVar",
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
    FLY_DEBUG_MESSAGE("CodeGenModule", "GenFunction",
                      "Function=" << Function->str() << ", isExternal=" << isExternal);
    CodeGenFunction *CGF = new CodeGenFunction(this, Function, isExternal);
    Function->setCodeGen(CGF);
    return CGF;
}

CodeGenClass *CodeGenModule::GenClass(ASTClass *Class, bool isExternal) {
    FLY_DEBUG_MESSAGE("CodeGenModule", "GenClass",
                      "Class=" << Class->str() << ", isExternal=" << isExternal);
    CodeGenClass *CGC = new CodeGenClass(this, Class, isExternal);
    Class->setCodeGen(CGC);
    CGC->Generate();
    return CGC;
}

void CodeGenModule::GenEnum(ASTEnum *Enum) {
    for (auto &Entry : Enum->getEntries()) {
        Entry->setCodeGen(new CodeGenEnumEntry(this, Entry));
    }
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
                    return Int32Ty;
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
            return llvm::ConstantAggregateZero::get(Ty);

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
            ASTIntegerValue *IntegerValue = (ASTIntegerValue *) Val;
            uint64_t IntValue = IntegerValue->getValue();
            switch (IntegerType->getIntegerKind()) {
                case ASTIntegerTypeKind::TYPE_BYTE:
                    return llvm::ConstantInt::get(Int8Ty, IntValue, false);
                case ASTIntegerTypeKind::TYPE_USHORT:
                    return llvm::ConstantInt::get(Int16Ty, IntValue, false);
                case ASTIntegerTypeKind::TYPE_SHORT:
                    return llvm::ConstantInt::get(Int16Ty, IntegerValue->isNegative() ? -IntValue: IntValue, true);
                case ASTIntegerTypeKind::TYPE_UINT:
                    return llvm::ConstantInt::get(Int32Ty, IntValue, false);
                case ASTIntegerTypeKind::TYPE_INT:
                    return llvm::ConstantInt::get(Int32Ty, IntegerValue->isNegative() ? -IntValue: IntValue, true);
                case ASTIntegerTypeKind::TYPE_ULONG:
                    return llvm::ConstantInt::get(Int64Ty, IntValue, false);
                case ASTIntegerTypeKind::TYPE_LONG:
                    return llvm::ConstantInt::get(Int64Ty, IntegerValue->isNegative() ? -IntValue: IntValue, true);
            }
        }

        // Floating Point
        case ASTTypeKind::TYPE_FLOATING_POINT: {
            ASTFloatingPointType *FPType = (ASTFloatingPointType *) Type;
            const std::string &FPValue = ((ASTFloatingValue *) Val)->getValue();
            switch (FPType->getFloatingPointKind()) {
                case ASTFloatingPointTypeKind::TYPE_FLOAT:
                    return llvm::ConstantFP::get(FloatTy, FPValue);
                case ASTFloatingPointTypeKind::TYPE_DOUBLE:
                    return llvm::ConstantFP::get(DoubleTy, FPValue);
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


//llvm::Value *CodeGenModule::Convert(llvm::Value *V, llvm::Type *T) {
//    if (V->getType()->isIntegerTy() && T->isIntegerTy()) {
//        if (V->getType()->getIntegerBitWidth() < T->getIntegerBitWidth()) {
//            return Builder->CreateZExt(V, T);
//        } else if (V->getType()->getIntegerBitWidth() > T->getIntegerBitWidth()) {
//            return Builder->CreateTrunc(V, T);
//        } else {
//            return V;
//        }
//    } else if (V->getType()->isIntegerTy() && T->isFloatingPointTy()) {
//        return V->getType()->getTypeID()
//    } else if (V->getType()->isFloatingPointTy() && T->isIntegerTy()) {
//
//    } else if (V->getType()->isFloatingPointTy() && T->isFloatingPointTy()) {
//        if (V->getType()->getFPMantissaWidth() < T->getFPMantissaWidth()) {
//            return Builder->CreateFPExt(V, T);
//        } else if (V->getType()->getFPMantissaWidth() > T->getFPMantissaWidth()) {
//            return Builder->CreateFPTrunc(V, T);
//        } else {
//            return V;
//        }
//    }
//}

llvm::Value *CodeGenModule::ConvertToBool(llvm::Value *V) {
    FLY_DEBUG_MESSAGE("CodeGenExpr", "Convert",
                      "FromVal=" << V << " to Bool Type=");
    if (V->getType()->isIntegerTy()) {
        if (V->getType()->getIntegerBitWidth() > 8) {
            llvm::Value *ZERO = llvm::ConstantInt::get(V->getType(), 0);
            return Builder->CreateICmpNE(V, ZERO);
        } else {
            return Builder->CreateTrunc(V, BoolTy);
        }
    }
    if (V->getType()->isFloatingPointTy()) {
        llvm::Value *ZERO = llvm::ConstantFP::get(V->getType(), 0);
        return Builder->CreateFCmpUNE(V, ZERO);
    }
    if (V->getType()->isArrayTy()) {
        // TODO
        return nullptr;
    }
    if (V->getType()->isStructTy()) {
        // TODO
        return nullptr;
    }

    assert(false && "Unhandled Value Type");
}

llvm::Value *CodeGenModule::Convert(llvm::Value *FromVal, const ASTType *FromType, const ASTType *ToType) {
    FLY_DEBUG_MESSAGE("CodeGenExpr", "Convert",
                      "Value=" << FromVal << " to ASTType=" << ToType->str());
    assert(ToType && "Invalid conversion type");

    llvm::Type *FromLLVMType = FromVal->getType();
    switch (ToType->getKind()) {

        // to BOOL
        case ASTTypeKind::TYPE_BOOL: {

            // from BOOL
            if (FromType->isBool()) {
                return Builder->CreateTrunc(FromVal, BoolTy);
            }

            // from Integer
            if (FromType->isInteger()) {
                llvm::Value *ZERO = llvm::ConstantInt::get(FromLLVMType, 0, ((ASTIntegerType *) FromType)->isSigned());
                return Builder->CreateICmpNE(FromVal, ZERO);
            }

            // from FLOATING POINT
            if (FromLLVMType->isFloatTy()) {
                llvm::Value *ZERO = llvm::ConstantFP::get(FromLLVMType, 0);
                return Builder->CreateFCmpUNE(FromVal, ZERO);
            }

            // default 0
            return llvm::ConstantInt::get(BoolTy, 0, false);
        }

            // to INTEGER
        case ASTTypeKind::TYPE_INTEGER: {
            ASTIntegerType *IntegerType = (ASTIntegerType *) ToType;
            switch(IntegerType->getIntegerKind()) {

                // to INT 8
                case ASTIntegerTypeKind::TYPE_BYTE: {

                    // from BOOL
                    if (FromType->isBool()) {
                        llvm::Value *ToVal = Builder->CreateTrunc(FromVal, BoolTy);
                        return Builder->CreateZExt(ToVal, Int8Ty);
                    }

                    // from INTEGER
                    if (FromType->isInteger()) {
                        if (FromLLVMType == Int8Ty) {
                            return FromVal;
                        } else {
                            return Builder->CreateTrunc(FromVal, Int8Ty);
                        }
                    }

                    // from FLOATING POINT
                    if (FromLLVMType->isFloatingPointTy()) {
                        return Builder->CreateFPToUI(FromVal, Int8Ty);
                    }
                }

                    // to INT 16
                case ASTIntegerTypeKind::TYPE_SHORT:
                case ASTIntegerTypeKind::TYPE_USHORT: {

                    // from BOOL
                    if (FromType->isBool()) {
                        llvm::Value *ToVal = Builder->CreateTrunc(FromVal, BoolTy);
                        return Builder->CreateZExt(ToVal, Int16Ty);
                    }

                    // from INTEGER
                    if (FromType->isInteger()) {
                        if (FromLLVMType == Int8Ty) {
                            return Builder->CreateZExt(FromVal, Int16Ty);
                        } else if (FromLLVMType == Int16Ty) {
                            return FromVal;
                        } else {
                            return Builder->CreateTrunc(FromVal, Int16Ty);
                        }
                    }

                    // from FLOATING POINT
                    if (FromLLVMType->isFloatingPointTy()) {
                        return IntegerType->isSigned() ? Builder->CreateFPToSI(FromVal, Int16Ty) :
                               Builder->CreateFPToUI(FromVal, Int16Ty);
                    }
                }

                    // to INT 32
                case ASTIntegerTypeKind::TYPE_INT:
                case ASTIntegerTypeKind::TYPE_UINT: {

                    // from BOOL
                    if (FromType->isBool()) {
                        llvm::Value *ToVal = Builder->CreateTrunc(FromVal, BoolTy);
                        return Builder->CreateZExt(ToVal, Int32Ty);
                    }

                    // from INTEGER
                    if (FromType->isInteger()) {
                        if (FromLLVMType == Int8Ty || FromLLVMType == Int16Ty) {
                            return IntegerType->isSigned() ? Builder->CreateSExt(FromVal, Int32Ty) :
                                   Builder->CreateZExt(FromVal, Int32Ty);
                        } else if (FromLLVMType == Int32Ty) {
                            return FromVal;
                        } else {
                            return Builder->CreateTrunc(FromVal, Int32Ty);
                        }
                    }

                    // from FLOATING POINT
                    if (FromLLVMType->isFloatingPointTy()) {
                        return IntegerType->isSigned() ? Builder->CreateFPToSI(FromVal, Int32Ty) :
                               Builder->CreateFPToUI(FromVal, Int32Ty);
                    }
                }

                    // to INT 64
                case ASTIntegerTypeKind::TYPE_LONG:
                case ASTIntegerTypeKind::TYPE_ULONG: {

                    // from BOOL
                    if (FromType->isBool()) {
                        llvm::Value *ToVal = Builder->CreateTrunc(FromVal, BoolTy);
                        return Builder->CreateZExt(ToVal, Int64Ty);
                    }

                    // from INTEGER
                    if (FromType->isInteger()) {
                        if (FromLLVMType == Int8Ty || FromLLVMType == Int16Ty ||
                            FromLLVMType == Int32Ty) {
                            return IntegerType->isSigned() ? Builder->CreateSExt(FromVal, Int64Ty) :
                                   Builder->CreateZExt(FromVal, Int64Ty);
                        } else {
                            return FromVal;
                        }
                    }

                    // from FLOATING POINT
                    if (FromType->isFloatingPoint()) {
                        return IntegerType->isSigned() ? Builder->CreateFPToSI(FromVal, Int64Ty) :
                               Builder->CreateFPToUI(FromVal, Int64Ty);
                    }
                }
            }
        }

            // to FLOATING POINT
        case ASTTypeKind::TYPE_FLOATING_POINT: {
            switch(((ASTFloatingPointType *) ToType)->getFloatingPointKind()) {

                // to FLOAT 32
                case ASTFloatingPointTypeKind::TYPE_FLOAT: {

                    // from BOOL
                    if (FromType->isBool()) {
                        return Builder->CreateTrunc(FromVal, BoolTy);
                    }

                    // from INT
                    if (FromType->isInteger()) {
                        return ((ASTIntegerType *) FromType)->isSigned() ?
                               Builder->CreateSIToFP(FromVal, FloatTy) :
                               Builder->CreateUIToFP(FromVal, FloatTy);
                    }

                    // from FLOAT
                    if (FromType->isFloatingPoint()) {
                        switch (((ASTFloatingPointType *) FromType)->getFloatingPointKind()) {

                            case ASTFloatingPointTypeKind::TYPE_FLOAT:
                                return FromVal;
                            case ASTFloatingPointTypeKind::TYPE_DOUBLE:
                                return Builder->CreateFPTrunc(FromVal, FloatTy);
                        }
                    }
                }

                    // to DOUBLE 64
                case ASTFloatingPointTypeKind::TYPE_DOUBLE: {

                    // from BOOL
                    if (FromType->isBool()) {
                        return Builder->CreateTrunc(FromVal, BoolTy);
                    }

                    // from INT
                    if (FromType->isInteger()) {
                        return ((ASTIntegerType *) FromType)->isSigned() ?
                               Builder->CreateSIToFP(FromVal, DoubleTy) :
                               Builder->CreateUIToFP(FromVal, DoubleTy);
                    }

                    // from FLOAT
                    if (FromType->isFloatingPoint()) {
                        switch (((ASTFloatingPointType *) FromType)->getFloatingPointKind()) {

                            case ASTFloatingPointTypeKind::TYPE_FLOAT:
                                return Builder->CreateFPExt(FromVal, DoubleTy);
                            case ASTFloatingPointTypeKind::TYPE_DOUBLE:
                                return FromVal;
                        }
                    }
                }
            }
        }

            // to Identity
        case ASTTypeKind::TYPE_IDENTITY:
            return FromVal; // TODO implement class cast
    }
    assert(0 && "Conversion failed");
}

CodeGenError *CodeGenModule::GenErrorHandler(ASTVar *Var) {
    // Set CodeGenError
    llvm::Value *Pointer = Builder->CreateAlloca(ErrorPtrTy);
    CodeGenError *CGE = new CodeGenError(this, Var, Pointer);
    return CGE;
}

CodeGenVar *CodeGenModule::GenLocalVar(ASTLocalVar *Var) {
    llvm::Type *Ty = GenType(Var->getType());
    CodeGenVar *CGV = new CodeGenVar(this, Ty);
    return CGV;
}

llvm::Value *CodeGenModule::GenVarRef(ASTVarRef *VarRef) {

    // Class Var
    if (VarRef->getDef()->getVarKind() == ASTVarKind::VAR_CLASS) {

        // Return the instance value
        if (VarRef->getParent()) {
            if (VarRef->getParent()->isCall()) { // TODO iterative parents
                // TODO
            } else if (VarRef->getParent()->isVarRef()) {
                CodeGenVarBase *CGI = ((ASTVarRef *) VarRef->getParent())->getDef()->getCodeGen();
                llvm::Value *Inst = CGI->getValue();
                return VarRef->getDef()->getCodeGen()->getValue();
            } else {
                // Error
            }
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

    // The function arguments
    llvm::SmallVector<llvm::Value *, 8> Args;
    // Add error as first param

    if (Call->getDef()->getKind() == ASTFunctionKind::CLASS_METHOD) {
        ASTClassMethod *Def = (ASTClassMethod *) Call->getDef();
        if (Def->getClass()->getClassKind() != ASTClassKind::STRUCT)
            Args.push_back(Call->getErrorHandler()->getCodeGen()->getValue()); // Error is a Pointer
    } else {
        Args.push_back(Call->getErrorHandler()->getCodeGen()->getValue()); // Error is a Pointer
    }

    // Take the CGI Value and pass to Call as first argument
    llvm::Value *Instance = nullptr;

    if (Call->getDef()->getKind() == ASTFunctionKind::CLASS_METHOD) {
        ASTClassMethod *Def = (ASTClassMethod *) Call->getDef();

        if (Call->getParent()) {
            if (Call->getParent()->isCall()) { // TODO iterative parents
                // TODO
            } else if (Call->getParent()->isVarRef()) {
                CodeGenVarBase *CGI = ((ASTVarRef *) Call->getParent())->getDef()->getCodeGen();
                Args.push_back(CGI->Load());
            }
        } else if (Def->isConstructor()) { // Call class constructor
            llvm::Type *AllocType = Def->getClass()->getAttributes().empty() ? Int8Ty : (llvm::Type *) Def->getClass()->getCodeGen()->getType();
            llvm::Constant *AllocSize = ConstantExpr::getTruncOrBitCast(ConstantExpr::getSizeOf(AllocType), AllocType);

            // @malloc data type struct
            IntegerType *IntPtrType = llvm::Type::getIntNTy(LLVMCtx, Module->getDataLayout().getMaxPointerSizeInBits());
            llvm::Instruction *I = CallInst::CreateMalloc(Builder->GetInsertBlock(), IntPtrType,
                                                          AllocType, AllocSize, nullptr, nullptr);
            Instance = Builder->Insert(I);
            Args.push_back(Instance);
        } else {
            // call static method
        }
    }

    // Add Call arguments to Function args
    for (ASTArg *Arg : Call->getArgs()) {
        llvm::Value *V = GenExpr(Arg->getExpr());
        Args.push_back(V);
    }
    llvm::Value *RetVal = Builder->CreateCall(Call->getDef()->getCodeGen()->getFunction(), Args);

    return Instance == nullptr ? RetVal : Instance;
}

llvm::Value *CodeGenModule::GenExpr(ASTExpr *Expr) {
    FLY_DEBUG("CodeGenModule", "GenExpr");
    CodeGenExpr *CGExpr = new CodeGenExpr(this, Expr);
    return CGExpr->getValue();
}

void CodeGenModule::GenStmt(CodeGenFunctionBase *CGF, ASTStmt * Stmt) {
    FLY_DEBUG("CodeGenModule", "GenStmt");
    switch (Stmt->getKind()) {

        // Var Assignment
        case ASTStmtKind::STMT_VAR: {
            ASTVarStmt *VarStmt = (ASTVarStmt *) Stmt;

            ASTVarRef *VarRef = VarStmt->getVarRef();

            if (VarStmt->getExpr()) {
                llvm::Value *V = GenExpr(VarStmt->getExpr()); // The Value represents the Expr result
                if (VarRef->getParent()) {
                    if (VarRef->getParent()->isCall()) { // TODO iterative parents
                        // TODO
                    } else if (VarRef->getParent()->isVarRef()) {
                        // Take Parent Instance
                        CodeGenVarBase *CGI = ((ASTVarRef *) VarRef->getParent())->getDef()->getCodeGen();
                        CodeGenVarBase *CGV = CGI->getVar(VarRef->getName()); // Get Var Name
                        CGV->Store(V);
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
            GenBlock(CGF, Block);
            break;
        }

        case ASTStmtKind::STMT_IF:
            GenIfBlock(CGF, (ASTIfStmt *)Stmt);
            break;

        case ASTStmtKind::STMT_SWITCH:
            GenSwitchBlock(CGF, (ASTSwitchStmt *)Stmt);
            break;

        case ASTStmtKind::STMT_LOOP: {
            GenLoopBlock(CGF, (ASTLoopStmt *) Stmt);
            break;
        }

        case ASTStmtKind::STMT_LOOP_IN: {
            break;
        }

            // Delete Stmt
        case ASTStmtKind::STMT_DELETE: {
            ASTDeleteStmt *Delete = (ASTDeleteStmt *) Stmt;
            ASTVar * Var = Delete->getVarRef()->getDef();
            if (Var->getType()->getKind() == ASTTypeKind::TYPE_IDENTITY) {
                Instruction *I = CallInst::CreateFree(Var->getCodeGen()->Load(), Builder->GetInsertBlock());
                Builder->Insert(I);
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
            GenReturn(Return->getFunction(), Return->getExpr());
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
            if (FailStmt->getExpr() == nullptr || FailStmt->getExpr()->getExprKind() == ASTExprKind::EXPR_EMPTY) {
                CGE->StoreDefault();
            } else if (FailStmt->getExpr()->getType()->isBool() || FailStmt->getExpr()->getType()->isInteger()) {
                llvm::Value *V = GenExpr(FailStmt->getExpr());
                CGE->StoreInt(V);
            } else if (FailStmt->getExpr()->getType()->isString()) {
                llvm::Value *V = GenExpr(FailStmt->getExpr());
                CGE->StoreString(V);
            } else if (FailStmt->getExpr()->getType()->isIdentity()) {
                ASTIdentityType * IdentityType = (ASTIdentityType *) FailStmt->getExpr()->getType();
                llvm::Value *V = GenExpr(FailStmt->getExpr());
                if (IdentityType->isEnum()) {
                    CGE->StoreInt(V);
                } else if (IdentityType->isClass()) {
                    CGE->StoreObject(V);
                }
            }
            

            // Generate Return with default value for stop execution flow
            if (FailStmt->hasHandle()) {
                ASTHandleStmt *HandleStmt = FailStmt->getHandle();
                HandleStmt->getCodeGen()->GoToBlock();
            } else {
                ASTFunctionBase *F = FailStmt->getParent()->getFunction();
                GenReturn(F);
            }
            break;
        }
    }
}

void CodeGenModule::GenBlock(CodeGenFunctionBase *CGF, ASTBlockStmt *BlockStmt) {
    FLY_DEBUG("CodeGenModule", "GenBlock");
    for (ASTStmt *Stmt : BlockStmt->getContent()) {
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
            Builder->SetInsertPoint(IfBB);
            GenStmt(CGF, If->getStmt());
            Builder->CreateBr(EndBB);
        } else { // If - elsif ...
            llvm::BasicBlock *ElsifBB = llvm::BasicBlock::Create(LLVMCtx, "elsif", Fn, EndBB);
            Builder->CreateCondBr(IfCond, IfBB, ElsifBB);

            // Start if-then
            Builder->SetInsertPoint(IfBB);
            GenStmt(CGF, If->getStmt());
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

                Builder->SetInsertPoint(ElsifThenBB);
                GenStmt(CGF, Elsif->getStmt());
                Builder->CreateBr(EndBB);

                ElsifBB = NextElsifBB;
            }
        }

    } else {

        // Create Else block
        llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(LLVMCtx, "else", Fn, EndBB);

        if (If->getElsif().empty()) { // If - Else
            Builder->CreateCondBr(IfCond, IfBB, ElseBB);
            Builder->SetInsertPoint(IfBB);
            GenStmt(CGF, If->getStmt());
            Builder->CreateBr(EndBB);
        } else { // If - Elsif - Else
            llvm::BasicBlock *ElsifBB = llvm::BasicBlock::Create(LLVMCtx, "elsif", Fn, ElseBB);
            Builder->CreateCondBr(IfCond, IfBB, ElsifBB);

            // Start if-then
            Builder->SetInsertPoint(IfBB);
            GenStmt(CGF, If->getStmt());
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

                Builder->SetInsertPoint(ElsifThenBB);
                GenStmt(CGF, Elsif->getStmt());
                Builder->CreateBr(EndBB);

                ElsifBB = NextElsifBB;
            }
        }

        Builder->SetInsertPoint(ElseBB);
        GenStmt(CGF, If->getElse());
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
        Builder->SetInsertPoint(ElsifThenBB);
        GenStmt(CGF, Elsif->getStmt());
        return GenElsifBlock(CGF, ElsifThenBB, It);
    }
}

void CodeGenModule::GenSwitchBlock(CodeGenFunctionBase *CGF, ASTSwitchStmt *Switch) {
    FLY_DEBUG("CodeGenModule", "GenSwitchBlock");
    llvm::Function *Fn = CGF->getFunction();

    // Create End Block
    llvm::BasicBlock *EndBB = llvm::BasicBlock::Create(LLVMCtx, "endswitch", Fn);

    // Create Expression evaluator for Switch
    llvm::Value *SwitchVal = Switch->getVarRef()->getDef()->getCodeGen()->getValue();
    llvm::SwitchInst *Inst = Builder->CreateSwitch(SwitchVal, EndBB);

    // Create Cases
    unsigned long Size = Switch->getCases().size();

    llvm::BasicBlock *NextCaseBB = nullptr;
    for (int i=0; i < Size; i++) {
        ASTSwitchCase *Case = Switch->getCases()[i];
        llvm::Value *CaseVal = GenExpr(Case->getValueExpr());
        llvm::ConstantInt *CaseConst = llvm::cast<llvm::ConstantInt, llvm::Value>(CaseVal);
        llvm::BasicBlock *CaseBB = NextCaseBB == nullptr ?
                                   llvm::BasicBlock::Create(LLVMCtx, "case", Fn, EndBB) : NextCaseBB;
        Inst->addCase(CaseConst, CaseBB);
        Builder->SetInsertPoint(CaseBB);
        GenStmt(CGF, Case->getStmt());

        // If there is a Next
        if (i + 1 < Size) {
            NextCaseBB = llvm::BasicBlock::Create(LLVMCtx, "case", Fn, EndBB);
            Builder->CreateBr(NextCaseBB);
        } else {
            Builder->CreateBr(EndBB);
        }
    }

    // Create Default
    if (Switch->getDefault()) {
        llvm::BasicBlock *DefaultBB = llvm::BasicBlock::Create(LLVMCtx, "default", Fn, EndBB);
        Inst->setDefaultDest(DefaultBB);
        Builder->SetInsertPoint(DefaultBB);
        GenStmt(CGF, Switch->getDefault());
        Builder->CreateBr(EndBB);
    }

    // Continue insertions into End Branch
    Builder->SetInsertPoint(EndBB);
}

void CodeGenModule::GenLoopBlock(CodeGenFunctionBase *CGF, ASTLoopStmt *Loop) {
    FLY_DEBUG("CodeGenModule", "GenLoopBlock");
    llvm::Function *Fn = CGF->getFunction();

    // Generate Init Statements
    if (Loop->getInit()) {
        GenStmt(CGF, Loop->getInit());
    }

    // Create Condition Block
    llvm::BasicBlock *CondBB = nullptr;
    if (Loop->getCondition()) {
        CondBB = llvm::BasicBlock::Create(LLVMCtx, "loopcond", Fn);
    }

    // Create Loop Block
    llvm::BasicBlock *LoopBB = LoopBB = llvm::BasicBlock::Create(LLVMCtx, "loop", Fn);

    // Create Post Block
    llvm::BasicBlock *PostBB = nullptr;
    if (Loop->getPost()) {
        PostBB = llvm::BasicBlock::Create(LLVMCtx, "looppost", Fn);
    }

    // Create End Block
    llvm::BasicBlock *EndBB = llvm::BasicBlock::Create(LLVMCtx, "loopend", Fn);

    // Generate Code
    if (CondBB) {
        Builder->CreateBr(CondBB);

        // Create Condition
        Builder->SetInsertPoint(CondBB);
        llvm::Value *Cond = GenExpr(Loop->getCondition());
        Builder->CreateCondBr(Cond, LoopBB, EndBB);
    } else {
        Builder->CreateBr(LoopBB);
    }

    // Add to Loop
    Builder->SetInsertPoint(LoopBB);
    GenStmt(CGF, Loop->getLoop());
    if (PostBB) {
        Builder->CreateBr(PostBB);

        // Add to Post
        Builder->SetInsertPoint(PostBB);
        GenStmt(CGF, Loop->getPost());
        if (CondBB) {
            Builder->CreateBr(CondBB);
        } else {
            Builder->CreateBr(LoopBB);
        }
    } else if (CondBB) {
        Builder->CreateBr(CondBB);
    } else {
        Builder->CreateBr(LoopBB);
    }

    // Continue insertions into End Branch
    Builder->SetInsertPoint(EndBB);
}

void CodeGenModule::GenReturn(ASTFunctionBase *F, ASTExpr *Expr) {
    // Create the Value for return
    if (F->getReturnType()->isVoid()) {
        Builder->CreateRetVoid();
    } else {
        Value *Ret;
        if (Expr) {
            llvm::Value *V = GenExpr(Expr);
            Ret = Convert(V, Expr->getType(), F->getReturnType());
        } else {
            Ret = GenDefaultValue(F->getReturnType());
        }
        Builder->CreateRet(Ret);
    }
}

