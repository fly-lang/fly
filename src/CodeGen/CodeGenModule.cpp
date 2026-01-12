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
#include "Sema/SemaNameSpace.h"
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
#include "AST/ASTIdentifier.h"
#include "AST/ASTReturnStmt.h"
#include "AST/ASTClass.h"
#include "AST/ASTEnum.h"
#include "AST/ASTExprStmt.h"
#include "Basic/Debug.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Value.h"

#include <AST/ASTDeclStmt.h>
#include <AST/ASTExpr.h>
#include <AST/ASTLocalVar.h>
#include <AST/ASTType.h>
#include <AST/ASTVar.h>
#include <CodeGen/CharUnits.h>
#include <Sema/SemaValue.h>
#include <Sema/SemaCall.h>
#include <Sema/SemaClassAttribute.h>
#include <Sema/SemaClassType.h>
#include <Sema/SemaClassMethod.h>
#include <Sema/SemaEnumType.h>
#include <Sema/SemaEnumEntry.h>
#include <Sema/SemaErrorHandler.h>
#include <Sema/SemaFunction.h>
#include <Sema/SemaMemberVar.h>
#include <Sema/SemaModule.h>
#include <Sema/SemaValue.h>
#include <Sema/SemaNameSpace.h>
#include <llvm/IR/Instructions.h>

using namespace fly;

/// toCharUnitsFromBits - Convert a size in bits to a size in characters.
CharUnits toCharUnitsFromBits(int64_t BitSize) {
    return CharUnits::fromQuantity(BitSize / 8);
}

CodeGenModule::CodeGenModule(DiagnosticsEngine &Diags, SemaModule *Sema, llvm::LLVMContext &LLVMCtx,
                             TargetInfo &Target, CodeGenOptions &CGOpts) :
        Diags(Diags),
        Sema(Sema),
        Target(Target),
        Module(new llvm::Module(Sema->getName(), LLVMCtx)),
        LLVMCtx(LLVMCtx),
        Builder(new llvm::IRBuilder<>(LLVMCtx)),
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

	Zero = llvm::ConstantInt::get(Int32Ty, 0);

    ErrorTy = CodeGenError::GenErrorType(LLVMCtx);
    ErrorPtrTy = llvm::PointerType::get(ErrorTy, 0);

	// Add Dummy Global Variable which use the Error Type to be sure that the type is in top of the Module
	new llvm::GlobalVariable(*Module, ErrorTy, true, llvm::GlobalValue::ExternalLinkage,
		nullptr, "error");

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

	for (auto &Node : Sema->getNodes()) {
		switch (Node->getKind()) {

			case SemaKind::FUNCTION: {
				SemaFunction *Function = static_cast<SemaFunction *>(Node);
				GenFunction(Function);
			}
			break;
			case SemaKind::CLASS:
				GenClass(static_cast<SemaClassType *>(Node));
			break;
			case SemaKind::ENUM:
				GenEnum(static_cast<SemaEnumType *>(Node));
			break;
		}
	}

	// Generate Function Bodies
	for (auto &CGF : CGFunctions) {
		CurrentFunction = CGF->getSema();
		CGF->GenBody();
	}

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

CodeGenFunction *CodeGenModule::GenFunction(SemaFunction *Sema, bool isExternal) {
    // FLY_DEBUG_START("CodeGenModule", "GenFunction",
    //                   "Function=" << Function->str() << ", isExternal=" << isExternal);
    CodeGenFunction *CGF = new CodeGenFunction(this, Sema, isExternal);
    Sema->setCodeGen(CGF);
	CGFunctions.push_back(CGF);
    return CGF;
}

CodeGenClass *CodeGenModule::GenClass(SemaClassType *Class, bool isExternal) {
    // FLY_DEBUG_START("CodeGenModule", "GenClass",
    //                   "Class=" << Class->str() << ", isExternal=" << isExternal);
    CodeGenClass *CGC = new CodeGenClass(this, Class, isExternal);
    Class->setCodeGen(CGC);
    return CGC;
}

void CodeGenModule::GenEnum(SemaEnumType *Enum) {
    for (auto &EntryEntry : Enum->getEntries()) {
    	SemaEnumEntry *Entry = EntryEntry.getValue();
        Entry->setCodeGen(new CodeGenEnumEntry(this, Entry));
    }
}

llvm::Type *CodeGenModule::GenType(SemaType *Type) {
    FLY_DEBUG_START("CodeGenModule", "GenType");
    // Check Type
    switch (Type->getTypeKind()) {

        case SemaTypeKind::TYPE_VOID:
            return VoidTy;

        case SemaTypeKind::TYPE_BOOL:
            return BoolTy;

        case SemaTypeKind::TYPE_INTEGER: {
            SemaIntTypeKind IntKind = static_cast<SemaIntType *>(Type)->getIntKind();
            switch (IntKind) {
                case SemaIntTypeKind::TYPE_BYTE:
                    return Int8Ty;
                case SemaIntTypeKind::TYPE_USHORT:
                case SemaIntTypeKind::TYPE_SHORT:
                    return Int16Ty;
                case SemaIntTypeKind::TYPE_UINT:
                case SemaIntTypeKind::TYPE_INT:
                    return Int32Ty;
                case SemaIntTypeKind::TYPE_ULONG:
                case SemaIntTypeKind::TYPE_LONG:
                    return Int64Ty;
            }
        }

        case SemaTypeKind::TYPE_FLOATING_POINT: {
            SemaFloatTypeKind FPKind = static_cast<SemaFloatType *>(Type)->getFPKind();
            switch (FPKind) {
                case SemaFloatTypeKind::TYPE_FLOAT:
                    return FloatTy;
                case SemaFloatTypeKind::TYPE_DOUBLE:
                    return DoubleTy;
            }
        }

        case SemaTypeKind::TYPE_ARRAY: {
            return GenArrayType(static_cast<SemaArrayType *>(Type));
        }

        case SemaTypeKind::TYPE_STRING: {
            return llvm::ArrayType::get(Int8Ty, 0);
        }

    	case SemaTypeKind::TYPE_ERROR: {
        	return ErrorTy;
    	}

        case SemaTypeKind::TYPE_CLASS: {
            SemaClassType *Class = static_cast<SemaClassType *>(Type);
            return Class->getCodeGen()->getType();
        }

		case SemaTypeKind::TYPE_ENUM:
			return Int32Ty;
    }
    assert(0 && "Unknown Var Type Kind");
}

llvm::PointerType *CodeGenModule::GenArrayType(SemaArrayType *ArrayType) {
    llvm::Type *SubType = GenType(ArrayType->getType());

	// TODO replace IntPtrType with IntPtrTy ?
    // Check if the Value is a ConstantInt
	llvm::IntegerType *IntPtrType = llvm::Type::getIntNTy(LLVMCtx, Module->getDataLayout().getMaxPointerSizeInBits());
	return llvm::PointerType::getUnqual(IntPtrType);
}

llvm::Constant *CodeGenModule::GenDefaultValue(SemaType *Type, llvm::Type *Ty) {
    FLY_DEBUG_START("CodeGenModule", "GenDefaultValue");
    assert(Type->getTypeKind() != SemaTypeKind::TYPE_VOID && "No default value for Void Type");
    switch (Type->getTypeKind()) {

        // Bool
        case SemaTypeKind::TYPE_BOOL:
            return llvm::ConstantInt::get(BoolTy, 0, false);

        // Integer
        case SemaTypeKind::TYPE_INTEGER: {
            SemaIntType *IntegerType = static_cast<SemaIntType *>(Type);
            switch (IntegerType->getIntKind()) {
                case SemaIntTypeKind::TYPE_BYTE:
                    return llvm::ConstantInt::get(Int8Ty, 0, false);
                case SemaIntTypeKind::TYPE_USHORT:
                    return llvm::ConstantInt::get(Int32Ty, 0, false);
                case SemaIntTypeKind::TYPE_SHORT:
                    return llvm::ConstantInt::get(Int32Ty, 0, true);
                case SemaIntTypeKind::TYPE_UINT:
                    return llvm::ConstantInt::get(Int32Ty, 0, false);
                case SemaIntTypeKind::TYPE_INT:
                    return llvm::ConstantInt::get(Int32Ty, 0, true);
                case SemaIntTypeKind::TYPE_ULONG:
                    return llvm::ConstantInt::get(Int64Ty, 0, false);
                case SemaIntTypeKind::TYPE_LONG:
                    return llvm::ConstantInt::get(Int64Ty, 0, true);
            }
        }

        // Floating Point
        case SemaTypeKind::TYPE_FLOATING_POINT: {
            SemaFloatType *FloatingPointType = static_cast<SemaFloatType *>(Type);
            switch (FloatingPointType->getFPKind()) {
                case SemaFloatTypeKind::TYPE_FLOAT:
                    return llvm::ConstantFP::get(FloatTy, 0.0);
                case SemaFloatTypeKind::TYPE_DOUBLE:
                    return llvm::ConstantFP::get(DoubleTy, 0.0);
            }
        }

        case SemaTypeKind::TYPE_ARRAY:
            return llvm::ConstantAggregateZero::get(Ty);

        case SemaTypeKind::TYPE_CLASS:
            return nullptr; // TODO
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
    FLY_DEBUG_START_MSG("CodeGenExpr", "Convert",
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

llvm::Value *CodeGenModule::Convert(llvm::Value *FromVal, SemaType *FromType, SemaType *ToType) {
    // FLY_DEBUG_START("CodeGenExpr", "Convert",
    //                   "Value=" << FromVal << " to ASTType=" << ToType->str());
    assert(ToType && "Invalid conversion type");

    llvm::Type *FromLLVMType = FromVal->getType();
    switch (ToType->getTypeKind()) {

        // to BOOL
        case SemaTypeKind::TYPE_BOOL: {

            // from BOOL
            if (FromType->isBool()) {
                return Builder->CreateTrunc(FromVal, BoolTy);
            }

            // from Integer
            if (FromType->isInteger()) {
                llvm::Value *ZERO = llvm::ConstantInt::get(FromLLVMType, 0, ((SemaIntType *) FromType)->isSigned());
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
        case SemaTypeKind::TYPE_INTEGER: {
            SemaIntType *IntegerType = (SemaIntType *) ToType;
            switch(IntegerType->getIntKind()) {

                // to INT 8
                case SemaIntTypeKind::TYPE_BYTE: {

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
                case SemaIntTypeKind::TYPE_SHORT:
                case SemaIntTypeKind::TYPE_USHORT: {

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
                case SemaIntTypeKind::TYPE_INT:
                case SemaIntTypeKind::TYPE_UINT: {

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
                case SemaIntTypeKind::TYPE_LONG:
                case SemaIntTypeKind::TYPE_ULONG: {

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
        case SemaTypeKind::TYPE_FLOATING_POINT: {
            switch(((SemaFloatType *) ToType)->getFPKind()) {

                // to FLOAT 32
                case SemaFloatTypeKind::TYPE_FLOAT: {

                    // from BOOL
                    if (FromType->isBool()) {
                        return Builder->CreateTrunc(FromVal, BoolTy);
                    }

                    // from INT
                    if (FromType->isInteger()) {
                        return ((SemaIntType *) FromType)->isSigned() ?
                               Builder->CreateSIToFP(FromVal, FloatTy) :
                               Builder->CreateUIToFP(FromVal, FloatTy);
                    }

                    // from FLOAT
                    if (FromType->isFloatingPoint()) {
                        switch (((SemaFloatType *) FromType)->getFPKind()) {

                            case SemaFloatTypeKind::TYPE_FLOAT:
                                return FromVal;
                            case SemaFloatTypeKind::TYPE_DOUBLE:
                                return Builder->CreateFPTrunc(FromVal, FloatTy);
                        }
                    }
                }

                    // to DOUBLE 64
                case SemaFloatTypeKind::TYPE_DOUBLE: {

                    // from BOOL
                    if (FromType->isBool()) {
                        return Builder->CreateTrunc(FromVal, BoolTy);
                    }

                    // from INT
                    if (FromType->isInteger()) {
                        return ((SemaIntType *) FromType)->isSigned() ?
                               Builder->CreateSIToFP(FromVal, DoubleTy) :
                               Builder->CreateUIToFP(FromVal, DoubleTy);
                    }

                    // from FLOAT
                    if (FromType->isFloatingPoint()) {
                        switch (((SemaFloatType *) FromType)->getFPKind()) {

                            case SemaFloatTypeKind::TYPE_FLOAT:
                                return Builder->CreateFPExt(FromVal, DoubleTy);
                            case SemaFloatTypeKind::TYPE_DOUBLE:
                                return FromVal;
                        }
                    }
                }
            }
        }

            // to Identity
        case SemaTypeKind::TYPE_CLASS:
            return FromVal; // TODO implement class cast
    }
    assert(0 && "Conversion failed");
}

CodeGenError *CodeGenModule::GenErrorHandler(SemaVar *Sema) {
    // Set CodeGenError
    llvm::Value *ErrorHandler = Builder->CreateAlloca(ErrorPtrTy);
    CodeGenError *CGE = new CodeGenError(this, Sema, ErrorHandler);
	Sema->setCodeGen(CGE);
    return CGE;
}

llvm::Value *CodeGenModule::GenExpr(SemaExpr *Expr) {
    FLY_DEBUG_START("CodeGenModule", "GenExpr");
	CodeGenExpr CGE(this);
	return CGE.GenExpr(Expr);
}

SemaNameSpace *CodeGenModule::getNameSpace() const {
	FLY_DEBUG_START("CodeGenModule", "getNameSpace");
	return Sema->getNameSpace();
}

std::string CodeGenModule::toIdentifier(llvm::StringRef Name, SemaNameSpace *NameSpace) {
	FLY_DEBUG_START("CodeGenModule", "toIdentifier");
	std::string Prefix = NameSpace ? std::string(NameSpace->getName()).append("_") : "";
	return Prefix.append(std::string(Name));
}

void CodeGenModule::GenStmt(CodeGenFunctionBase *CGF, ASTStmt * Stmt) {
    FLY_DEBUG_START("CodeGenModule", "GenStmt");
    switch (Stmt->getStmtKind()) {

    	case ASTStmtKind::STMT_DECL: {
    		ASTDeclStmt *DeclStmt = static_cast<ASTDeclStmt *>(Stmt);

    		// Declaration may be with initialization
    		if (DeclStmt->getExpr()) {
    			GenExpr(DeclStmt->getExpr()->getSema());
    		}
    	}
    	break;

        // Expression Statement (includes assignments, calls, etc.)
        case ASTStmtKind::STMT_EXPR: {
            ASTExprStmt *ExprStmt = static_cast<ASTExprStmt *>(Stmt);
            GenExpr(ExprStmt->getExpr()->getSema());
            break;
        }

        // Block of Stmt
        case ASTStmtKind::STMT_BLOCK: {
            ASTBlockStmt *Block = static_cast<ASTBlockStmt *>(Stmt);
            GenBlockStmt(CGF, Block);
            break;
        }

        case ASTStmtKind::STMT_IF:
            GenIfStmt(CGF, static_cast<ASTIfStmt *>(Stmt));
            break;

        case ASTStmtKind::STMT_SWITCH:
            GenSwitchStmt(CGF, static_cast<ASTSwitchStmt *>(Stmt));
            break;

        case ASTStmtKind::STMT_LOOP: {
            GenLoopStmt(CGF, static_cast<ASTLoopStmt *>(Stmt));
            break;
        }

        case ASTStmtKind::STMT_LOOP_IN: {
        	// TODO: GenLoopInStmt()
            break;
        }

            // Delete Stmt
        case ASTStmtKind::STMT_DELETE: {
            ASTDeleteStmt *Delete = static_cast<ASTDeleteStmt *>(Stmt);
        	llvm::Value *V = GenExpr(Delete->getExpr()->getSema());
            if (Delete->getExpr()->getType()->isClass()) {
                llvm::Instruction *I = llvm::CallInst::CreateFree(V, Builder->GetInsertBlock());
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
            ASTReturnStmt *Return = static_cast<ASTReturnStmt *>(Stmt);
            GenReturn(Return->getFunction(), Return->getExpr());
            break;
        }

        case ASTStmtKind::STMT_HANDLE: {
            ASTHandleStmt *HandleStmt = static_cast<ASTHandleStmt *>(Stmt);

        	// Set Handle Block
            llvm::BasicBlock *HandleBB = llvm::BasicBlock::Create(LLVMCtx, "handle", CGF->getFunction());
            Builder->CreateBr(HandleBB);
            Builder->SetInsertPoint(HandleBB);

        	// Generate Handle Block
            GenStmt(CGF, HandleStmt->getHandle());

        	// Generate in Safe Block
        	llvm::BasicBlock *SafeBB = llvm::BasicBlock::Create(LLVMCtx, "safe", CGF->getFunction());
        	Builder->SetInsertPoint(SafeBB);
        	CurrentFunction->getCodeGen()->setSafeBB(SafeBB);
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
        			CodeGenError *CGE = CurrentFunction->getErrorHandler()->getCodeGen();
        			GenFailStmt(FailStmt, CGE);

        			// Generate Return with default value for stop execution flow
        			ASTFunction *F = FailStmt->getParent()->getFunction();
        			GenReturn(F);
        			break;
        		} else if (Parent->getStmtKind() == ASTStmtKind::STMT_HANDLE) {
					// Set ErrorHandler of the parent with Fail
					ASTHandleStmt * HandleStmt = static_cast<ASTHandleStmt *>(Parent);

					// Take the current ErrorHandler CodeGen (already resolved in ResolveStmtHandle())
					CodeGenError *CGE = static_cast<CodeGenError *>(static_cast<SemaVar *>(HandleStmt->getErrorHandler()->getSema())->getCodeGen());
					GenFailStmt(FailStmt, CGE);

					Builder->CreateBr(CurrentFunction->getCodeGen()->getSafeBB());
        			break;
				}
        	}
        	break;
        }
    }
}

void CodeGenModule::GenFailStmt(ASTFailStmt *FailStmt, CodeGenError *CGE) {
	// Store Fail value in ErrorHandler
	if (FailStmt->getExpr() == nullptr) {
		CGE->StoreInt(llvm::ConstantInt::get(Int32Ty, 1));
	} else if (FailStmt->getExpr()->getType()->isBool() || FailStmt->getExpr()->getType()->isInteger()) {
		llvm::Value *V = GenExpr(FailStmt->getExpr()->getSema());
		CGE->StoreInt(V);
	} else if (FailStmt->getExpr()->getType()->isString()) {
		llvm::Value *V = GenExpr(FailStmt->getExpr()->getSema());
		CGE->StoreString(V);
	} else if (FailStmt->getExpr()->getType()->isClass()) {
		SemaType * IdentityType = FailStmt->getExpr()->getType();
		llvm::Value *V = GenExpr(FailStmt->getExpr()->getSema());
		CGE->StoreObject(V);
	} else if (FailStmt->getExpr()->getType()->isEnum()) {
		SemaType * IdentityType = FailStmt->getExpr()->getType();
		llvm::Value *V = GenExpr(FailStmt->getExpr()->getSema());
		CGE->StoreInt(V);
	}
}

void CodeGenModule::GenBlockStmt(CodeGenFunctionBase *CGF, ASTBlockStmt *BlockStmt) {
    FLY_DEBUG_START("CodeGenModule", "GenBlock");
    for (ASTStmt *Stmt : BlockStmt->getContent()) {
        GenStmt(CGF, Stmt);
    }
}

void CodeGenModule::GenIfStmt(CodeGenFunctionBase *CGF, ASTIfStmt *If) {
    FLY_DEBUG_START("CodeGenModule", "GenIfBlock");
    llvm::Function *Fn = CGF->getFunction();

    // If Block
    llvm::Value *IfCond = GenExpr(If->getRule()->getSema());
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
                llvm::Value *ElsifCond = GenExpr(Elsif->getRule()->getSema());
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
                llvm::Value *ElsifCond = GenExpr(Elsif->getRule()->getSema());
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

void CodeGenModule::GenElsifStmt(CodeGenFunctionBase *CGF,
                                               llvm::BasicBlock *ElsifBB,
                                               llvm::SmallVector<ASTRuleStmt *, 8>::iterator &It) {
    FLY_DEBUG_START("CodeGenModule", "GenElsifBlock");
    llvm::Function *Fn = CGF->getFunction();
    ASTRuleStmt *&Elsif = *It;
    It++;
    if (*It != nullptr) {
        Builder->SetInsertPoint(ElsifBB);
        llvm::Value *Cond = GenExpr(Elsif->getRule()->getSema());
        llvm::BasicBlock *NextElsifBB = llvm::BasicBlock::Create(LLVMCtx, "elsif", Fn);
        Builder->CreateCondBr(Cond, ElsifBB, NextElsifBB);

        llvm::BasicBlock *ElsifThenBB = llvm::BasicBlock::Create(LLVMCtx, "elsifthen", Fn);
        Builder->SetInsertPoint(ElsifThenBB);
        GenStmt(CGF, Elsif->getStmt());
        GenElsifStmt(CGF, ElsifThenBB, It);
    }
}

void CodeGenModule::GenSwitchStmt(CodeGenFunctionBase *CGF, ASTSwitchStmt *Switch) {
    FLY_DEBUG_START("CodeGenModule", "GenSwitchBlock");
    llvm::Function *Fn = CGF->getFunction();

    // Create End Block
    llvm::BasicBlock *EndBB = llvm::BasicBlock::Create(LLVMCtx, "endswitch", Fn);

    // Create Expression evaluator for Switch
    llvm::Value *SwitchVal = GenExpr(Switch->getVar()->getSema());
    llvm::SwitchInst *Inst = Builder->CreateSwitch(SwitchVal, EndBB);

    // Create Cases
    unsigned long Size = Switch->getCases().size();

    llvm::BasicBlock *NextCaseBB = nullptr;
    for (unsigned long i=0; i < Size; i++) {
        ASTRuleStmt *Case = Switch->getCases()[i];
        llvm::Value *CaseVal = GenExpr(Case->getRule()->getSema());
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

void CodeGenModule::GenLoopStmt(CodeGenFunctionBase *CGF, ASTLoopStmt *Loop) {
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
        llvm::Value *Cond = GenExpr(Loop->getRule()->getSema());
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
    // Create the Value for return // FIXME
    if (F->getReturnType()->getSema()->isVoid()) {
        Builder->CreateRetVoid();
    } else {
        llvm::Value *Ret;
        if (Expr) {
            llvm::Value *V = GenExpr(Expr->getSema());
            Ret = Convert(V, Expr->getType(), F->getReturnType()->getSema());
        } else {
            Ret = GenDefaultValue(F->getReturnType()->getSema());
        }
        Builder->CreateRet(Ret);
    }
}

