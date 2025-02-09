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
#include "CodeGen/CodeGenError.h"
#include "CodeGen/CodeGenHandle.h"
#include "Sym/SymNameSpace.h"
#include "AST/ASTModule.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTDeleteStmt.h"
#include "AST/ASTArg.h"
#include "AST/ASTCall.h"
#include "AST/ASTFunction.h"
#include "AST/ASTFailStmt.h"
#include "AST/ASTHandleStmt.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTIfStmt.h"
#include "AST/ASTSwitchStmt.h"
#include "AST/ASTLoopStmt.h"
#include "AST/ASTValue.h"
#include "AST/ASTAssignmentStmt.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTReturnStmt.h"
#include "AST/ASTClass.h"
#include "AST/ASTEnum.h"
#include "AST/ASTExprStmt.h"
#include "Basic/Debug.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Value.h"

#include <AST/ASTTypeRef.h>
#include <AST/ASTVar.h>
#include <CodeGen/CharUnits.h>
#include <Sym/SymClass.h>
#include <Sym/SymEnum.h>
#include <Sym/SymEnumEntry.h>
#include <Sym/SymFunction.h>
#include <Sym/SymGlobalVar.h>

using namespace llvm;
using namespace fly;

/// toCharUnitsFromBits - Convert a size in bits to a size in characters.
CharUnits toCharUnitsFromBits(int64_t BitSize) {
    return CharUnits::fromQuantity(BitSize / 8);
}

CodeGenModule::CodeGenModule(DiagnosticsEngine &Diags, SymNameSpace &NameSpace, LLVMContext &LLVMCtx,
                             TargetInfo &Target, CodeGenOptions &CGOpts) :
        Diags(Diags),
        NameSpace(NameSpace),
        Target(Target),
        Module(new llvm::Module(NameSpace.getName(), LLVMCtx)),
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

SymNameSpace &CodeGenModule::getNameSpace() const {
    return NameSpace;
}

void CodeGenModule::GenHeaders() {
    GenAll();
}

void CodeGenModule::GenAll() {

    // TODO
    // Generate External GlobalVars
//    for (const auto &Entry : AST.getExternalGlobalVars()) {
//        ASTGlobalVar *GlobalVar = Entry.getValue();
//        FLY_DEBUG_MESSAGE("FrontendAction", "GenerateCode",
//                          "ExternalGlobalVar=" << GlobalVar->str());
//        GenGlobalVar(GlobalVar, true);
//    }
//
//    // Generate External Function
//    for (auto &StrMapEntry : AST.getExternalFunctions()) {
//        for (auto &IntMap : StrMapEntry.getValue()) {
//            for (auto &Function : IntMap.second) {
//                FLY_DEBUG_MESSAGE("FrontendAction", "GenerateCode",
//                                  "ExternalFunction=" << Function->str());
//                GenFunction(Function, true);
//            }
//        }
//    }

    // Generate GlobalVars
    std::vector<CodeGenGlobalVar *> CGGlobalVars;
    for (const auto GlobalVarEntry : NameSpace.getGlobalVars()) {
    	SymGlobalVar *GlobalVar = GlobalVarEntry.getValue();
        CodeGenGlobalVar *CGV = GenGlobalVar(GlobalVar);
        CGGlobalVars.push_back(CGV);
//        if (FrontendOpts.CreateHeader) {
//            CGH->AddGlobalVar(GlobalVar);
//        }
    }

    // Instantiates all Function CodeGen in order to be set in all Call references
    std::vector<CodeGenFunction *> CGFunctions;
    for (auto Func : NameSpace.getFunctions()) {
        CodeGenFunction *CGF = GenFunction(Func);
        CGFunctions.push_back(CGF);
//                if (FrontendOpts.CreateHeader) {
//                    CGH->AddFunction(Func);
//                }
    }

	// Generate Classes
	llvm::SmallVector<CodeGenClass *, 8> CGClasses;
	for (auto ClassEntry : NameSpace.getClasses()) {
			CodeGenClass *CGC = GenClass(ClassEntry.getValue());
			CGClasses.push_back(CGC);
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

	// Generate Enums
	for (auto EnumEntry : NameSpace.getEnums()) {
		GenEnum(EnumEntry.getValue());
	}

}

/**
 * GenStmt from VarDecl
 * @param GlobalVar
 * @param isExternal
 */
CodeGenGlobalVar *CodeGenModule::GenGlobalVar(SymGlobalVar* GlobalVar, bool isExternal) {
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

CodeGenFunction *CodeGenModule::GenFunction(SymFunction *Function, bool isExternal) {
    FLY_DEBUG_MESSAGE("CodeGenModule", "GenFunction",
                      "Function=" << Function->str() << ", isExternal=" << isExternal);
    CodeGenFunction *CGF = new CodeGenFunction(this, Function, isExternal);
    Function->setCodeGen(CGF);
    return CGF;
}

CodeGenClass *CodeGenModule::GenClass(SymClass *Class, bool isExternal) {
    FLY_DEBUG_MESSAGE("CodeGenModule", "GenClass",
                      "Class=" << Class->str() << ", isExternal=" << isExternal);
    CodeGenClass *CGC = new CodeGenClass(this, Class, isExternal);
    Class->setCodeGen(CGC);
    CGC->Generate();
    return CGC;
}

void CodeGenModule::GenEnum(SymEnum *Enum) {
    for (auto EntryEntry : Enum->getEntries()) {
    	SymEnumEntry *Entry = EntryEntry.getValue();
        Entry->setCodeGen(new CodeGenEnumEntry(this, Entry));
    }
}

llvm::Type *CodeGenModule::GenType(const SymType *Type) {
    FLY_DEBUG_START("CodeGenModule", "GenType");
    // Check Type
    switch (Type->getKind()) {

        case SymTypeKind::TYPE_VOID:
            return VoidTy;

        case SymTypeKind::TYPE_BOOL:
            return BoolTy;

        case SymTypeKind::TYPE_INTEGER: {
            SymIntTypeKind IntKind = ((SymTypeInt *) Type)->getIntKind();
            switch (IntKind) {
                case SymIntTypeKind::TYPE_BYTE:
                    return Int8Ty;
                case SymIntTypeKind::TYPE_USHORT:
                case SymIntTypeKind::TYPE_SHORT:
                    return Int16Ty;
                case SymIntTypeKind::TYPE_UINT:
                case SymIntTypeKind::TYPE_INT:
                    return Int32Ty;
                case SymIntTypeKind::TYPE_ULONG:
                case SymIntTypeKind::TYPE_LONG:
                    return Int64Ty;
            }
        }

        case SymTypeKind::TYPE_FLOATING_POINT: {
            SymFPTypeKind FPKind = ((SymTypeFP *) Type)->getFPKind();
            switch (FPKind) {
                case SymFPTypeKind::TYPE_FLOAT:
                    return FloatTy;
                case SymFPTypeKind::TYPE_DOUBLE:
                    return DoubleTy;
            }
        }

        case SymTypeKind::TYPE_ARRAY: {
            return GenArrayType((SymTypeArray *) Type);
        }

        case SymTypeKind::TYPE_STRING: {
            return llvm::ArrayType::get(Int8Ty, 0);
        }

        case SymTypeKind::TYPE_CLASS: {
            SymClass *Class = (SymClass *) Type;
            return Class->getCodeGen()->getType();
        }

		case SymTypeKind::TYPE_ENUM:
			return Int32Ty;

        case SymTypeKind::TYPE_ERROR: {
            return ErrorTy;
        }
    }
    assert(0 && "Unknown Var Type Kind");
}

llvm::ArrayType *CodeGenModule::GenArrayType(const SymTypeArray *ArrayType) {
    llvm::Type *SubType = GenType(ArrayType->getType());
    llvm::Value *Val = GenExpr(ArrayType->getSize());

    // Check if the Value is a ConstantInt
    if (auto *constInt = dyn_cast<ConstantInt>(Val)) {
        // Extract the value as uint64_t
        uint64_t IntVal = constInt->getZExtValue(); // Use getSExtValue() for signed values
        return llvm::ArrayType::get(SubType, IntVal);
    }

    // TODO Error: cannot convert Size to Int

    return nullptr;
    assert("Array Size error");
}

llvm::Constant *CodeGenModule::GenDefaultValue(const SymType *Type, llvm::Type *Ty) {
    FLY_DEBUG_START("CodeGenModule", "GenDefaultValue");
    assert(Type->getStmtKind() != ASTValueKind::TYPE_VOID && "No default value for Void Type");
    switch (Type->getStmtKind()) {

        // Bool
        case ASTValueKind::TYPE_BOOL:
            return llvm::ConstantInt::get(BoolTy, 0, false);

        // Integer
        case ASTValueKind::TYPE_INTEGER: {
            ASTIntegerType *IntegerType = (ASTIntegerType *) Type;
            switch (IntegerType->getIntegerKind()) {
                case SymIntTypeKind::TYPE_BYTE:
                    return llvm::ConstantInt::get(Int8Ty, 0, false);
                case SymIntTypeKind::TYPE_USHORT:
                    return llvm::ConstantInt::get(Int32Ty, 0, false);
                case SymIntTypeKind::TYPE_SHORT:
                    return llvm::ConstantInt::get(Int32Ty, 0, true);
                case SymIntTypeKind::TYPE_UINT:
                    return llvm::ConstantInt::get(Int32Ty, 0, false);
                case SymIntTypeKind::TYPE_INT:
                    return llvm::ConstantInt::get(Int32Ty, 0, true);
                case SymIntTypeKind::TYPE_ULONG:
                    return llvm::ConstantInt::get(Int64Ty, 0, false);
                case SymIntTypeKind::TYPE_LONG:
                    return llvm::ConstantInt::get(Int64Ty, 0, true);
            }
        }

        // Floating Point
        case ASTValueKind::TYPE_FLOATING_POINT: {
            ASTFloatingPointType *FloatingPointType = (ASTFloatingPointType *) Type;
            switch (FloatingPointType->getFloatingPointKind()) {
                case SymFPTypeKind::TYPE_FLOAT:
                    return llvm::ConstantFP::get(FloatTy, 0.0);
                case SymFPTypeKind::TYPE_DOUBLE:
                    return llvm::ConstantFP::get(DoubleTy, 0.0);
            }
        }

        case ASTValueKind::TYPE_ARRAY:
            return llvm::ConstantAggregateZero::get(Ty);

        case ASTValueKind::TYPE_CLASS:
            return nullptr; // TODO
    }
    assert(0 && "Unknown Type");
}

/**
 * Generate a LLVM Constant Value
 * @param Type is the parsed symType
 * @param Val need to be correctly configured or you need to call GenDefaultValue()
 * @return
 */
llvm::Constant *CodeGenModule::GenValue(const SymType *Type, const ASTValue *Val) {
    FLY_DEBUG_START("CodeGenModule", "GenValue");
    assert(Type && "Type has to be not empty");
    assert(Val && "Value has to be not empty");

    //TODO value conversion from Val->getType() to TypeBase (if are different)

    switch (Type->getStmtKind()) {

        // Bool
        case ASTValueKind::TYPE_BOOL:
            return llvm::ConstantInt::get(BoolTy, ((ASTBoolValue *)Val)->getValue(), false);

        // Integer
        case ASTValueKind::TYPE_INTEGER: {
            ASTIntegerType *IntegerType = (ASTIntegerType *) Type;
            ASTIntegerValue *IntegerValue = (ASTIntegerValue *) Val;
            switch (IntegerType->getIntegerKind()) {
                case SymIntTypeKind::TYPE_BYTE:
                    return llvm::ConstantInt::get(Int8Ty, IntegerValue->getValue(), IntegerValue->getRadix());
                case SymIntTypeKind::TYPE_USHORT:
                    return llvm::ConstantInt::get(Int16Ty, IntegerValue->getValue(), IntegerValue->getRadix());
                case SymIntTypeKind::TYPE_SHORT:
                    return llvm::ConstantInt::get(Int16Ty, IntegerValue->getValue(), IntegerValue->getRadix());
                case SymIntTypeKind::TYPE_UINT:
                    return llvm::ConstantInt::get(Int32Ty, IntegerValue->getValue(), IntegerValue->getRadix());
                case SymIntTypeKind::TYPE_INT:
                    return llvm::ConstantInt::get(Int32Ty, IntegerValue->getValue(), IntegerValue->getRadix());
                case SymIntTypeKind::TYPE_ULONG:
                    return llvm::ConstantInt::get(Int64Ty, IntegerValue->getValue(), IntegerValue->getRadix());
                case SymIntTypeKind::TYPE_LONG:
                    return llvm::ConstantInt::get(Int64Ty, IntegerValue->getValue(), IntegerValue->getRadix());
            }
        }

        // Floating Point
        case ASTValueKind::TYPE_FLOATING_POINT: {
            ASTFloatingPointType *FPType = (ASTFloatingPointType *) Type;
            const std::string &FPValue = ((ASTFloatingValue *) Val)->getValue();
            switch (FPType->getFloatingPointKind()) {
                case SymFPTypeKind::TYPE_FLOAT:
                    return llvm::ConstantFP::get(FloatTy, FPValue);
                case SymFPTypeKind::TYPE_DOUBLE:
                    return llvm::ConstantFP::get(DoubleTy, FPValue);
            }
        }

        // Array
        case ASTValueKind::TYPE_ARRAY: {
            llvm::ArrayType *ArrType = GenArrayType((ASTArrayType *) Type);
            std::vector<llvm::Constant *> Values;
            for (ASTValue *Value : ((ASTArrayValue *) Val)->getValues()) {
                llvm::Constant * V = GenValue(((ASTArrayType *) Type)->getType(), Value);
                Values.push_back(V);
            }
            return llvm::ConstantArray::get(ArrType, makeArrayRef(Values));
        }

        case ASTValueKind::TYPE_STRING: {
            return Builder->CreateGlobalStringPtr(((ASTStringValue *) Val)->getValue());
        }

        // Identity
        case ASTValueKind::TYPE_CLASS:
            break;

        // Void
        case ASTValueKind::TYPE_VOID:
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

llvm::Value *CodeGenModule::Convert(llvm::Value *FromVal, const SymType *FromType, const SymType *ToType) {
    FLY_DEBUG_MESSAGE("CodeGenExpr", "Convert",
                      "Value=" << FromVal << " to ASTType=" << ToType->str());
    assert(ToType && "Invalid conversion type");

    llvm::Type *FromLLVMType = FromVal->getType();
    switch (ToType->getStmtKind()) {

        // to BOOL
        case ASTValueKind::TYPE_BOOL: {

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
        case ASTValueKind::TYPE_INTEGER: {
            ASTIntegerType *IntegerType = (ASTIntegerType *) ToType;
            switch(IntegerType->getIntegerKind()) {

                // to INT 8
                case SymIntTypeKind::TYPE_BYTE: {

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
                case SymIntTypeKind::TYPE_SHORT:
                case SymIntTypeKind::TYPE_USHORT: {

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
                case SymIntTypeKind::TYPE_INT:
                case SymIntTypeKind::TYPE_UINT: {

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
                case SymIntTypeKind::TYPE_LONG:
                case SymIntTypeKind::TYPE_ULONG: {

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
        case ASTValueKind::TYPE_FLOATING_POINT: {
            switch(((ASTFloatingPointType *) ToType)->getFloatingPointKind()) {

                // to FLOAT 32
                case SymFPTypeKind::TYPE_FLOAT: {

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

                            case SymFPTypeKind::TYPE_FLOAT:
                                return FromVal;
                            case SymFPTypeKind::TYPE_DOUBLE:
                                return Builder->CreateFPTrunc(FromVal, FloatTy);
                        }
                    }
                }

                    // to DOUBLE 64
                case SymFPTypeKind::TYPE_DOUBLE: {

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

                            case SymFPTypeKind::TYPE_FLOAT:
                                return Builder->CreateFPExt(FromVal, DoubleTy);
                            case SymFPTypeKind::TYPE_DOUBLE:
                                return FromVal;
                        }
                    }
                }
            }
        }

            // to Identity
        case ASTValueKind::TYPE_CLASS:
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

CodeGenVar *CodeGenModule::GenLocalVar(ASTVar *Var) {
    llvm::Type *Ty = GenType(Var->getTypeRef()->getDef());
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
    FLY_DEBUG_MESSAGE("CodeGenModule", "GenCall", "Call=" << Call->str());
    assert(Call->getDef() && "Undefined Call");

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

    SymFunctionBase *Def = Call->getDef();
    if (!Def) {
        Diag(Call->getLocation(), diag::err_cg_unresolved_def) << Call->getName();
    }
    CodeGenFunctionBase *CGF = Def->getCodeGen();
    llvm::Value *RetVal = Builder->CreateCall(CGF->getFunction(), Args);

    return Instance == nullptr ? RetVal : Instance;
}

llvm::Value *CodeGenModule::GenExpr(ASTExpr *Expr) {
    FLY_DEBUG_START("CodeGenModule", "GenExpr");
    CodeGenExpr *CGExpr = new CodeGenExpr(this, Expr);
    return CGExpr->getValue();
}

void CodeGenModule::GenFailStmt(ASTFailStmt *FailStmt, CodeGenError *CGE) {
	// Store Fail value in ErrorHandler
	if (FailStmt->getExpr() == nullptr) {
		CGE->StoreInt(llvm::ConstantInt::get(Int32Ty, 1));
	} else if (FailStmt->getExpr()->getTypeRef()->getDef()->isBool() || FailStmt->getExpr()->getTypeRef()->getDef()->isInteger()) {
		llvm::Value *V = GenExpr(FailStmt->getExpr());
		CGE->StoreInt(V);
	} else if (FailStmt->getExpr()->getTypeRef()->getDef()->isString()) {
		llvm::Value *V = GenExpr(FailStmt->getExpr());
		CGE->StoreString(V);
	} else if (FailStmt->getExpr()->getTypeRef()->getDef()->isClass()) {
		ASTTypeRef * IdentityType = FailStmt->getExpr()->getTypeRef();
		llvm::Value *V = GenExpr(FailStmt->getExpr());
		CGE->StoreObject(V);
	} else if (FailStmt->getExpr()->getTypeRef()->getDef()->isEnum()) {
		ASTTypeRef * IdentityType = FailStmt->getExpr()->getTypeRef();
		llvm::Value *V = GenExpr(FailStmt->getExpr());
		CGE->StoreInt(V);
	}
}

void CodeGenModule::GenStmt(CodeGenFunctionBase *CGF, ASTStmt * Stmt) {
    FLY_DEBUG_START("CodeGenModule", "GenStmt");
    switch (Stmt->getStmtKind()) {

        // Var Assignment
        case ASTStmtKind::STMT_ASSIGN: {
            ASTAssignmentStmt *VarStmt = (ASTAssignmentStmt *) Stmt;

            ASTVarRef *VarRef = VarStmt->getVarRef();

            if (VarStmt->getExpr()) {
                llvm::Value *V = GenExpr(VarStmt->getExpr()); // The Value represents the Expr result
                if (VarRef->getParent()) {
                    if (VarRef->getParent()->isCall()) { // TODO iterative parents
                        // TODO
                    } else if (VarRef->getParent()->isVarRef()) {
                        // Take Parent Instance
                        CodeGenVarBase *CGI = static_cast<ASTVarRef *>(VarRef->getParent())->getDef()->getCodeGen();
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
            ASTExprStmt *ExprStmt = static_cast<ASTExprStmt *>(Stmt);
            GenExpr(ExprStmt->getExpr());
            break;
        }

            // Block of Stmt
        case ASTStmtKind::STMT_BLOCK: {
            ASTBlockStmt *Block = static_cast<ASTBlockStmt *>(Stmt);
            GenBlock(CGF, Block);
            break;
        }

        case ASTStmtKind::STMT_IF:
            GenIfBlock(CGF, static_cast<ASTIfStmt *>(Stmt));
            break;

        case ASTStmtKind::STMT_SWITCH:
            GenSwitchBlock(CGF, static_cast<ASTSwitchStmt *>(Stmt));
            break;

        case ASTStmtKind::STMT_LOOP: {
            GenLoopBlock(CGF, static_cast<ASTLoopStmt *>(Stmt));
            break;
        }

        case ASTStmtKind::STMT_LOOP_IN: {
            break;
        }

            // Delete Stmt
        case ASTStmtKind::STMT_DELETE: {
            ASTDeleteStmt *Delete = (ASTDeleteStmt *) Stmt;
            SymVar * Var = Delete->getVarRef()->getDef();
            if (Var->getAST()->getTypeRef()->getDef()->isClass()) {
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
            ASTHandleStmt *HandleStmt = static_cast<ASTHandleStmt *>(Stmt);

        	// New CodeGen Handle
        	CodeGenHandle *CGH = new CodeGenHandle(this, CGF);
        	HandleStmt->setCodeGen(CGH);

        	// Set Handle Block
            llvm::BasicBlock *HandleBB = CGH->getHandleBlock();
            Builder->CreateBr(HandleBB);
            Builder->SetInsertPoint(HandleBB);

        	// Continue with code generator
            GenStmt(CGF, HandleStmt->getHandle());

        	// Continue in Safe Block
        	Builder->SetInsertPoint(CGH->getSafeBlock());
            break;
        }

        case ASTStmtKind::STMT_FAIL: {
            ASTFailStmt *FailStmt = static_cast<ASTFailStmt *>(Stmt);

        	ASTStmt *Parent = FailStmt->getParent();

        	// Set error handler with parent block or function
        	while (true) {
        		Parent = Parent->getParent();
        		if (Parent == nullptr) {
        			// Set Function ErrorHandler with Fail
        			CodeGenError *CGE = static_cast<CodeGenError *>(FailStmt->getFunction()->getErrorHandler()->getCodeGen());
        			GenFailStmt(FailStmt, CGE);

        			// Generate Return with default value for stop execution flow
        			ASTFunction *F = FailStmt->getParent()->getFunction();
        			GenReturn(F);
        			break;
        		} else if (Parent->getStmtKind() == ASTStmtKind::STMT_HANDLE) {
					// Set ErrorHandler of the parent with Fail
					ASTHandleStmt * HandleStmt = static_cast<ASTHandleStmt *>(Parent);

					// Take the current ErrorHandler CodeGen (already resolved in ResolveStmtHandle())
					CodeGenError *CGE = (CodeGenError *) HandleStmt->getErrorHandlerRef()->getDef()->getCodeGen();
					GenFailStmt(FailStmt, CGE);

					CodeGenHandle *CGH = HandleStmt->getCodeGen();
					Builder->CreateBr(CGH->getSafeBlock());
        			break;
				}
        	}
        	break;
        }
    }
}

void CodeGenModule::GenBlock(CodeGenFunctionBase *CGF, ASTBlockStmt *BlockStmt) {
    FLY_DEBUG_START("CodeGenModule", "GenBlock");
    for (ASTStmt *Stmt : BlockStmt->getContent()) {
        GenStmt(CGF, Stmt);
    }
}

void CodeGenModule::GenIfBlock(CodeGenFunctionBase *CGF, ASTIfStmt *If) {
    FLY_DEBUG_START("CodeGenModule", "GenIfBlock");
    llvm::Function *Fn = CGF->getFunction();

    // If Block
    llvm::Value *IfCond = GenExpr(If->getRule());
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
                ASTRuleStmt *Elsif = If->getElsif()[i];
                Builder->SetInsertPoint(ElsifBB);
                llvm::Value *ElsifCond = GenExpr(Elsif->getRule());
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
                ASTRuleStmt *Elsif = If->getElsif()[i];
                Builder->SetInsertPoint(ElsifBB);
                llvm::Value *ElsifCond = GenExpr(Elsif->getRule());
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
                                               llvm::SmallVector<ASTRuleStmt *, 8>::iterator &It) {
    FLY_DEBUG_START("CodeGenModule", "GenElsifBlock");
    llvm::Function *Fn = CGF->getFunction();
    ASTRuleStmt *&Elsif = *It;
    It++;
    if (*It == nullptr) {
        return ElsifBB;
    } else {
        Builder->SetInsertPoint(ElsifBB);
        llvm::Value *Cond = GenExpr(Elsif->getRule());
        llvm::BasicBlock *NextElsifBB = llvm::BasicBlock::Create(LLVMCtx, "elsif", Fn);
        Builder->CreateCondBr(Cond, ElsifBB, NextElsifBB);

        llvm::BasicBlock *ElsifThenBB = llvm::BasicBlock::Create(LLVMCtx, "elsifthen", Fn);
        Builder->SetInsertPoint(ElsifThenBB);
        GenStmt(CGF, Elsif->getStmt());
        return GenElsifBlock(CGF, ElsifThenBB, It);
    }
}

void CodeGenModule::GenSwitchBlock(CodeGenFunctionBase *CGF, ASTSwitchStmt *Switch) {
    FLY_DEBUG_START("CodeGenModule", "GenSwitchBlock");
    llvm::Function *Fn = CGF->getFunction();

    // Create End Block
    llvm::BasicBlock *EndBB = llvm::BasicBlock::Create(LLVMCtx, "endswitch", Fn);

    // Create Expression evaluator for Switch
    llvm::Value *SwitchVal = Switch->getVarRef()->getDef()->getCodeGen()->getValue();
    llvm::SwitchInst *Inst = Builder->CreateSwitch(SwitchVal, EndBB);

    // Create Cases
    unsigned long Size = Switch->getCases().size();

    llvm::BasicBlock *NextCaseBB = nullptr;
    for (unsigned long i=0; i < Size; i++) {
        ASTRuleStmt *Case = Switch->getCases()[i];
        llvm::Value *CaseVal = GenExpr(Case->getRule());
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
    FLY_DEBUG_START("CodeGenModule", "GenLoopBlock");
    llvm::Function *Fn = CGF->getFunction();

    // Generate Init Statements
    if (Loop->getInit()) {
        GenStmt(CGF, Loop->getInit());
    }

    // Create Condition Block
    llvm::BasicBlock *CondBB = nullptr;
    if (Loop->getRule()) {
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
        llvm::Value *Cond = GenExpr(Loop->getRule());
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

void CodeGenModule::GenReturn(ASTFunction *F, ASTExpr *Expr) {
    // Create the Value for return
    if (F->getReturnTypeRef()->getDef()->isVoid()) {
        Builder->CreateRetVoid();
    } else {
        Value *Ret;
        if (Expr) {
            llvm::Value *V = GenExpr(Expr);
            Ret = Convert(V, Expr->getTypeRef()->getDef(), F->getReturnTypeRef()->getDef());
        } else {
            Ret = GenDefaultValue(F->getReturnTypeRef()->getDef());
        }
        Builder->CreateRet(Ret);
    }
}

