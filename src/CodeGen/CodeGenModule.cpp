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

#include "AST/ASTArg.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTCall.h"
#include "AST/ASTClass.h"
#include "AST/ASTDeleteStmt.h"
#include "AST/ASTEnum.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTFailStmt.h"
#include "AST/ASTFunction.h"
#include "AST/ASTHandleStmt.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTIfStmt.h"
#include "AST/ASTLoopStmt.h"
#include "AST/ASTModule.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTReturnStmt.h"
#include "AST/ASTSwitchStmt.h"
#include "Basic/Debug.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenClass.h"
#include "CodeGen/CodeGenEnumValue.h"
#include "CodeGen/CodeGenError.h"
#include "CodeGen/CodeGenExpr.h"
#include "CodeGen/CodeGenFunction.h"
#include "Sema/SemaBinary.h"
#include "Sema/SemaCast.h"
#include "Sema/SemaNameSpace.h"
#include "Sema/SemaParam.h"
#include "Sema/SemaTernary.h"
#include "Sema/SemaUnary.h"

#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Value.h"

#include <AST/ASTDeclStmt.h>
#include <AST/ASTExpr.h>
#include <AST/ASTLocalVar.h>
#include <AST/ASTType.h>
#include <AST/ASTVar.h>
#include <CodeGen/CharUnits.h>
#include <Sema/SemaCall.h>
#include <Sema/SemaClassAttribute.h>
#include <Sema/SemaClassMethod.h>
#include <Sema/SemaClassType.h>
#include <Sema/SemaEnumType.h>
#include <Sema/SemaEnumValue.h>
#include <Sema/SemaErrorHandler.h>
#include <Sema/SemaFunction.h>
#include <Sema/SemaMember.h>
#include <Sema/SemaModule.h>
#include <Sema/SemaNameSpace.h>
#include <Sema/SemaValue.h>
#include <llvm/IR/Instructions.h>

using namespace fly;

/// toCharUnitsFromBits - Convert a size in bits to a size in characters.
CharUnits toCharUnitsFromBits(int64_t BitSize) {
    return CharUnits::fromQuantity(BitSize / 8);
}

CodeGenModule::CodeGenModule(DiagnosticsEngine &Diags, StringRef Name, llvm::LLVMContext &LLVMCtx,
                             TargetInfo &Target, CodeGenOptions &CGOpts) :
        Diags(Diags),
        Target(Target),
        Module(new llvm::Module(Name, LLVMCtx)),
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


    // TODO Add dependencies, Linker Options
}

CodeGenModule::~CodeGenModule() {
    delete Module;
}

DiagnosticBuilder CodeGenModule::Diag(unsigned DiagID) {
    return Diags.Report(DiagID);
}

llvm::Module *CodeGenModule::getModule() const {
    return Module;
}

llvm::LLVMContext &CodeGenModule::getLLVMCtx() const {
	return LLVMCtx;
}

void CodeGenModule::GenBlockStmt(CodeGenFunctionBase *CGF, ASTBlockStmt *BlockStmt) {
	FLY_DEBUG_START("CodeGenModule", "GenBlock");
	for (ASTStmt *Stmt : BlockStmt->getContent()) {
		GenStmt(CGF, Stmt);
	}
}

void CodeGenModule::GenStmt(CodeGenFunctionBase *CGF, ASTStmt * Stmt) {
    FLY_DEBUG_START("CodeGenModule", "GenStmt");
    switch (Stmt->getStmtKind()) {

    	case ASTStmtKind::STMT_DECL: {
    		ASTDeclStmt *DeclStmt = static_cast<ASTDeclStmt *>(Stmt);

    		// Declaration may be with initialization
    		if (DeclStmt->getExpr()) {
    			DeclStmt->getExpr()->getSema()->accept(*this);
    		}
    	}
    	break;

        // Expression Statement (includes assignments, calls, etc.)
        case ASTStmtKind::STMT_EXPR: {
            ASTExprStmt *ExprStmt = static_cast<ASTExprStmt *>(Stmt);
            ExprStmt->getExpr()->getSema()->accept(*this);
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
        	Delete->getExpr()->getSema()->accept(*this);
        	llvm::Value *V = Delete->getExpr()->getSema()->getCodeGen()->getValue();
        	// Free Memory
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
        	ASTExpr *Expr = Return->getExpr();
        	if (Expr == nullptr) {
        		Builder->CreateRetVoid();
        	} else {
        		Expr->getSema()->accept(*this);
        		llvm::Value *V = Expr->getSema()->getCodeGen()->getValue();
        		Builder->CreateRet(V);
        	}
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
        			if (CurrentFunction->getReturnType()->isVoid()) {
        				Builder->CreateRetVoid();
        			} else {
        				CurrentFunction->getDefaultReturnValue()->accept(*this);
						llvm::Value *RetV = CurrentFunction->getDefaultReturnValue()->getCodeGen()->getValue();
						Builder->CreateRet(RetV);
					}
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
	} else {
		FailStmt->getExpr()->getSema()->accept(*this);
		llvm::Value *V = FailStmt->getExpr()->getSema()->getCodeGen()->getValue();
		if (FailStmt->getExpr()->getType()->isBool() || FailStmt->getExpr()->getType()->isInteger()) {
			CGE->StoreInt(V);
		} else if (FailStmt->getExpr()->getType()->isString()) {
			CGE->StoreString(V);
		} else if (FailStmt->getExpr()->getType()->isClass()) {
			llvm::Value *V = FailStmt->getExpr()->getSema()->getCodeGen()->getValue();
			CGE->StoreObject(V);
		} else if (FailStmt->getExpr()->getType()->isEnum()) {
			CGE->StoreInt(V);
		}
	}
}

void CodeGenModule::GenIfStmt(CodeGenFunctionBase *CGF, ASTIfStmt *If) {
    FLY_DEBUG_START("CodeGenModule", "GenIfBlock");
    llvm::Function *Fn = CGF->getFunction();

    // If Block
	If->getRule()->getSema()->accept(*this);
    llvm::Value *IfCond = If->getRule()->getSema()->getCodeGen()->getValue();
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
            	Elsif->getRule()->getSema()->accept(*this);
                llvm::Value *ElsifCond = Elsif->getRule()->getSema()->getCodeGen()->getValue();
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
            	Elsif->getRule()->getSema()->accept(*this);
                llvm::Value *ElsifCond = Elsif->getRule()->getSema()->getCodeGen()->getValue();
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
    	Elsif->getRule()->getSema()->accept(*this);
        llvm::Value *Cond = Elsif->getRule()->getSema()->getCodeGen()->getValue();
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
	Switch->getVar()->getSema()->accept(*this);
    llvm::Value *SwitchVal = Switch->getVar()->getSema()->getCodeGen()->getValue();
    llvm::SwitchInst *Inst = Builder->CreateSwitch(SwitchVal, EndBB);

    // Create Cases
    unsigned long Size = Switch->getCases().size();

    llvm::BasicBlock *NextCaseBB = nullptr;
    for (unsigned long i=0; i < Size; i++) {
        ASTRuleStmt *Case = Switch->getCases()[i];
    	Case->getRule()->getSema()->accept(*this);
        llvm::Value *CaseVal = Case->getRule()->getSema()->getCodeGen()->getValue();
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
    	Loop->getRule()->getSema()->accept(*this);
        llvm::Value *Cond = Loop->getRule()->getSema()->getCodeGen()->getValue();
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

std::string CodeGenModule::toIdentifier(llvm::StringRef Name, SemaNameSpace *NameSpace) {
	FLY_DEBUG_START("CodeGenModule", "toIdentifier");
	std::string Prefix = NameSpace ? std::string(NameSpace->getName()).append("_") : "";
	return Prefix.append(std::string(Name));
}

// =============================================================================
// SemaVisitor interface implementation
// =============================================================================

void CodeGenModule::visit(SemaModule &Sema) {
    // Generate all nodes (functions, classes, enums)
    for (auto &Node : Sema.getNodes()) {
        Node->accept(*this);
    }

    // Generate Function Bodies in a second pass
	for (auto &FB : Functions) {
		FB->accept(*this);
	}
}

void CodeGenModule::visit(SemaNameSpace &Sema) {
	// NameSpaces are not directly generated - they are organizational constructs
}

void CodeGenModule::visit(SemaImport &Sema) {
	// Imports are resolved during semantic analysis, not code generation
}

void CodeGenModule::visit(SemaBoolType &Sema) {
    if (Sema.getCodeGen() == nullptr) {
		CodeGenType *CG = new CodeGenType(this);
    	CG->GenType(Sema);
		Sema.setCodeGen(CG);
	}
}

void CodeGenModule::visit(SemaIntType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenType *CG = new CodeGenType(this);
		CG->GenType(Sema);
		Sema.setCodeGen(CG);
	}
}

void CodeGenModule::visit(SemaFloatType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenType *CG = new CodeGenType(this);
		CG->GenType(Sema);
		Sema.setCodeGen(CG);
	}
}

void CodeGenModule::visit(SemaArrayType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenType *CG = new CodeGenType(this);
		CG->GenType(Sema);
		Sema.setCodeGen(CG);
	}
}

void CodeGenModule::visit(SemaErrorType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenType *CG = new CodeGenType(this);
		CG->GenType(Sema);
		Sema.setCodeGen(CG);
	}
}

void CodeGenModule::visit(SemaVoidType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenType *CG = new CodeGenType(this);
		CG->GenType(Sema);
		Sema.setCodeGen(CG);
	}
}

void CodeGenModule::visit(SemaStringType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenType *CG = new CodeGenType(this);
		CG->GenType(Sema);
		Sema.setCodeGen(CG);
	}
}

void CodeGenModule::visit(SemaEnumType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		for (auto &EnumEntry : Sema.getEntries()) {
			SemaEnumValue *Sema = EnumEntry.getValue();
			CodeGenEnumValue *CGEV = new CodeGenEnumValue(this, Sema);
			Sema->setCodeGen(CGEV);
		}
	}
}

void CodeGenModule::visit(SemaClassType &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenClass *CGC = new CodeGenClass(this, &Sema, false);
		Sema.setCodeGen(CGC);
	}
}

void CodeGenModule::visit(SemaClassMethod &Sema) {
	CurrentFunction = &Sema;
	if (Sema.getCodeGen()) {
		Sema.getCodeGen()->GenBody();
	}
}

void CodeGenModule::visit(SemaFunction &Sema) {
	CurrentFunction = &Sema;
	if (Sema.getCodeGen() == nullptr) {
		CodeGenFunction *CGF = new CodeGenFunction(this, &Sema, false);
		Sema.setCodeGen(CGF);
		Functions.push_back(&Sema);
	} else {
		Sema.getCodeGen()->GenBody();
	}
}

void CodeGenModule::visit(SemaClassAttribute &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		Sema.getType()->accept(*this);
		llvm::Type *T = Sema.getType()->getCodeGen()->getType();
		CodeGenVar *CGV = new CodeGenVar(this, &Sema, T);
		Sema.setCodeGen(CGV);
	}
}

void CodeGenModule::visit(SemaLocalVar &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		Sema.getType()->accept(*this);
		llvm::Type *T = Sema.getType()->getCodeGen()->getType();
		CodeGenVar *CGV = new CodeGenVar(this, &Sema, T);
		Sema.setCodeGen(CGV);
	}
}

void CodeGenModule::visit(SemaParam &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		Sema.getType()->accept(*this);
		llvm::Type *T = Sema.getType()->getCodeGen()->getType();
		CodeGenVar *CGV = new CodeGenVar(this, &Sema, T);
		Sema.setCodeGen(CGV);
	}
}

void CodeGenModule::visit(SemaClassInstance &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		Sema.getType()->accept(*this);
		llvm::Type *T = Sema.getType()->getCodeGen()->getType();
		CodeGenVar *CGV = new CodeGenVar(this, &Sema, T);
		Sema.setCodeGen(CGV);
	}
}

void CodeGenModule::visit(SemaErrorHandler &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		Sema.getType()->accept(*this);
		llvm::Value *ErrorHandler = Builder->CreateAlloca(ErrorPtrTy);
		CodeGenError *CGE = new CodeGenError(this, &Sema, ErrorHandler);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaMember &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaCall &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaUnary &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaBinary &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaTernary &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaCast &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaBoolValue &Sema) {
    if (Sema.getCodeGen() == nullptr) {
    	CodeGenExpr *CGE = new CodeGenExpr(this);
    	CGE->GenExpr(&Sema);
    	Sema.setCodeGen(CGE);
    }
}

void CodeGenModule::visit(SemaIntValue &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaFloatValue &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaStringValue &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaArrayValue &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaStructValue &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaNullValue &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaEnumValue &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

