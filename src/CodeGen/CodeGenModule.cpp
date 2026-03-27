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
#include "AST/ASTBreakStmt.h"
#include "AST/ASTCall.h"
#include "AST/ASTClass.h"
#include "AST/ASTContinueStmt.h"
#include "AST/ASTDeleteStmt.h"
#include "AST/ASTEnum.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTFailStmt.h"
#include "AST/ASTFunction.h"
#include "AST/ASTHandleStmt.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTIfStmt.h"
#include "AST/ASTLoopInStmt.h"
#include "AST/ASTLoopStmt.h"
#include "AST/ASTModule.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTReturnStmt.h"
#include "AST/ASTSwitchStmt.h"
#include "Basic/Debug.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenArrayValue.h"
#include "CodeGen/CodeGenClass.h"
#include "CodeGen/CodeGenEnum.h"
#include "CodeGen/CodeGenEnumEntry.h"
#include "CodeGen/CodeGenError.h"
#include "CodeGen/CodeGenExpr.h"
#include "CodeGen/CodeGenFunction.h"
#include "Sema/SemaBinary.h"
#include "Sema/SemaCast.h"
#include "Sema/SemaNameSpace.h"
#include "Sema/SemaParam.h"
#include "Sema/SemaTernary.h"
#include "Sema/SemaUnary.h"
#include "Sema/SemaEnumEntry.h"

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
#include <Sema/SemaEnumEntry.h>
#include <Sema/SemaEnumList.h>
#include <Sema/SemaError.h>
#include <Sema/SemaFunction.h>
#include <Sema/SemaMember.h>
#include <Sema/SemaModule.h>
#include <Sema/SemaValue.h>
#include <llvm/IR/Instructions.h>

using namespace fly;

CodeGenModule::CodeGenModule(CodeGen &CG, DiagnosticsEngine &Diags, StringRef Name, llvm::LLVMContext &LLVMCtx,
                             TargetInfo &Target, CodeGenOptions &CGOpts) :
        CG(CG),
        Diags(Diags),
        Target(Target),
        LLVMCtx(LLVMCtx),
        Module(new llvm::Module(Name, LLVMCtx)),
        Builder(new llvm::IRBuilder<>(LLVMCtx)),
        CGOpts(CGOpts),
        CurrentFunction(nullptr) {

    // Types are now in CodeGen (CG), no need to initialize them here

	// Add Dummy Global Variable which use the Error Type to be sure that the type is in top of the Module
	new llvm::GlobalVariable(*Module, CG.ErrorTy, true, llvm::GlobalValue::ExternalLinkage,
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
    // Ensure stacks are clean (they should be empty at this point)
    BreakTargetStack.clear();
    ContinueTargetStack.clear();
    delete Builder;
    // Note: Module ownership is transferred to caller via getModule(), so we don't delete it here
}

DiagnosticBuilder CodeGenModule::Diag(unsigned DiagID) {
    return Diags.Report(DiagID);
}

llvm::Module *CodeGenModule::getModule() const {
    return Module;
}

TargetInfo &CodeGenModule::getTarget() {
	return Target;
}

llvm::LLVMContext &CodeGenModule::getLLVMCtx() const {
	return LLVMCtx;
}

llvm::IRBuilder<> *CodeGenModule::getBuilder() const {
	return Builder;
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
		CodeGenEnum *CGE = new CodeGenEnum(this, &Sema, false);
		Sema.setCodeGen(CGE);
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

void CodeGenModule::visit(SemaError &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		Sema.getType()->accept(*this);
		llvm::Value *ErrorHandler = Builder->CreateAlloca(CG.ErrorPtrTy);
		CodeGenError *CGE = new CodeGenError(this, &Sema, ErrorHandler);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaMember &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		// Only set wrapper if GenExpr didn't already set the CodeGen (e.g. CodeGenVar for attributes)
		if (Sema.getCodeGen() == nullptr) {
			Sema.setCodeGen(CGE);
		}
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
		CodeGenArrayValue *CGE = new CodeGenArrayValue(this);
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

void CodeGenModule::visit(SemaUnsetValue &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenExpr *CGE = new CodeGenExpr(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaEnumEntry &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenEnumEntry *CGE = new CodeGenEnumEntry(this, &Sema);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}

void CodeGenModule::visit(SemaEnumList &Sema) {
	if (Sema.getCodeGen() == nullptr) {
		CodeGenArrayValue *CGE = new CodeGenArrayValue(this);
		CGE->GenExpr(&Sema);
		Sema.setCodeGen(CGE);
	}
}


void CodeGenModule::GenBlockStmt(ASTBlockStmt *BlockStmt) {
	FLY_DEBUG_START("CodeGenModule", "GenBlock");
	for (ASTStmt *Stmt : BlockStmt->getContent()) {
		GenStmt(Stmt);
	}
}

void CodeGenModule::GenStmt(ASTStmt * Stmt) {
	FLY_DEBUG_START("CodeGenModule", "GenStmt");
	switch (Stmt->getStmtKind()) {

		case ASTStmtKind::STMT_DECL:
			GenDeclStmt(static_cast<ASTDeclStmt *>(Stmt));
			break;

			// Expression Statement (includes assignments, calls, etc.)
		case ASTStmtKind::STMT_EXPR:
			GenExprStmt(static_cast<ASTExprStmt *>(Stmt));
			break;

			// Block of Stmt
		case ASTStmtKind::STMT_BLOCK:
			GenBlockStmt(static_cast<ASTBlockStmt *>(Stmt));
			break;

		case ASTStmtKind::STMT_IF:
			GenIfStmt(static_cast<ASTIfStmt *>(Stmt));
			break;

		case ASTStmtKind::STMT_SWITCH:
			GenSwitchStmt(static_cast<ASTSwitchStmt *>(Stmt));
			break;

		case ASTStmtKind::STMT_LOOP:
			GenLoopStmt(static_cast<ASTLoopStmt *>(Stmt));
			break;

		case ASTStmtKind::STMT_LOOP_IN:
			GenStmtLoopIn(static_cast<ASTLoopInStmt *>(Stmt));
			break;

		// Delete Stmt
		case ASTStmtKind::STMT_DELETE:
			GenDeleteStmt(static_cast<ASTDeleteStmt *>(Stmt));
			break;

			// Break Stmt
		case ASTStmtKind::STMT_BREAK:
			GenBreakStmt(static_cast<ASTBreakStmt *>(Stmt));
			break;

			// Continue Stmt
		case ASTStmtKind::STMT_CONTINUE:
			GenContinueStmt(static_cast<ASTContinueStmt *>(Stmt));
			break;

			// Return Stmt
		case ASTStmtKind::STMT_RETURN:
				GenReturnStmt(static_cast<ASTReturnStmt *>(Stmt));
				break;

		case ASTStmtKind::STMT_HANDLE:
			GenHandleStmt(static_cast<ASTHandleStmt *>(Stmt));
			break;

		case ASTStmtKind::STMT_FAIL:
			GenFailStmt(static_cast<ASTFailStmt *>(Stmt));
			break;
	}
}

void CodeGenModule::GenDeclStmt(ASTDeclStmt *DeclStmt) {
	FLY_DEBUG_START("CodeGenModule", "GenStmtDecl");

	// Get the CodeGenVar for the Local Variable
	CodeGenVar *CGV = DeclStmt->getLocalVar()->getSema()->getCodeGen();

	// Declaration may be with initialization
	if (DeclStmt->getExpr()) {
		DeclStmt->getExpr()->getSema()->accept(*this);
	} else {
		CGV->StoreDefaultValue();
	}

	FLY_DEBUG_END("CodeGenModule", "GenStmtDecl");
}

void CodeGenModule::GenExprStmt(ASTExprStmt *ExprStmt) {
	FLY_DEBUG_START("CodeGenModule", "GenStmtExpr");

	ExprStmt->getExpr()->getSema()->accept(*this);

	FLY_DEBUG_END("CodeGenModule", "GenStmtExpr");
}

void CodeGenModule::GenDeleteStmt(ASTDeleteStmt *DeleteStmt) {
	FLY_DEBUG_START("CodeGenModule", "GenStmtDelete");

	DeleteStmt->getExpr()->getSema()->accept(*this);
	llvm::Value *V = DeleteStmt->getExpr()->getSema()->getCodeGen()->getValue();

	// Free Memory
	if (DeleteStmt->getExpr()->getType()->isClass()) {
		llvm::Instruction *I = llvm::CallInst::CreateFree(V, Builder->GetInsertBlock());
		Builder->Insert(I);
	}

	FLY_DEBUG_END("CodeGenModule", "GenStmtDelete");
}

void CodeGenModule::GenBreakStmt(ASTBreakStmt *BreakStmt) {
	FLY_DEBUG_START("CodeGenModule", "GenStmtBreak");

	// Break jumps to the end of the enclosing loop or switch
	if (!BreakTargetStack.empty()) {
		llvm::BasicBlock *BreakTarget = BreakTargetStack.back();
		Builder->CreateBr(BreakTarget);
	} else {
		// Error: break outside of loop/switch
		// TODO: Add specific diagnostic for break outside loop/switch
		Diag(diag::err_invalid_behavior);
	}

	FLY_DEBUG_END("CodeGenModule", "GenStmtBreak");
}

void CodeGenModule::GenContinueStmt(ASTContinueStmt *ContinueStmt) {
	FLY_DEBUG_START("CodeGenModule", "GenStmtContinue");

	// Continue jumps to the start of the next iteration (or condition) of the enclosing loop
	if (!ContinueTargetStack.empty()) {
		llvm::BasicBlock *ContinueTarget = ContinueTargetStack.back();
		Builder->CreateBr(ContinueTarget);
	} else {
		// Error: continue outside of loop
		// TODO: Add specific diagnostic for continue outside loop
		Diag(diag::err_invalid_behavior);
	}

	FLY_DEBUG_END("CodeGenModule", "GenStmtContinue");
}

void CodeGenModule::GenReturnStmt(ASTReturnStmt *ReturnStmt) {
	FLY_DEBUG_START("CodeGenModule", "GenStmtReturn");
	Builder->CreateRetVoid();
	FLY_DEBUG_END("CodeGenModule", "GenStmtReturn");
}

void CodeGenModule::GenHandleStmt(ASTHandleStmt *HandleStmt) {
	FLY_DEBUG_START("CodeGenModule", "GenStmtHandle");

	// Save parent error handler to restore after generating the current handle and safe blocks
	CodeGenError *ParentErrorHandler = CurrentErrorHandler;

	// Create a new error handler for this handle statement and set it as the current error handler
	HandleStmt->getErrorHandler()->accept(*this);
	CurrentErrorHandler = HandleStmt->getErrorHandler()->getCodeGen();

	// Save parent handle statement for nested handles
	llvm::BasicBlock *ParentHandleBB = CurrentHandleBB;
	llvm::BasicBlock *ParentSafeBB = CurrentSafeBB;

	// Take the current Function to create the Handle and Safe blocks in the same function
	llvm::Function *Fn = CurrentFunction->getCodeGen()->getFunction();

	// Create Handle and Safe Block
	CurrentHandleBB = llvm::BasicBlock::Create(LLVMCtx, "handle", Fn);
	CurrentSafeBB = llvm::BasicBlock::Create(LLVMCtx, "safe", Fn);
	Builder->CreateBr(CurrentHandleBB);
	Builder->SetInsertPoint(CurrentHandleBB);

	// Generate Handle Block
	GenStmt(HandleStmt->getHandle());

	// Generate in Safe Block
	Builder->SetInsertPoint(CurrentSafeBB);

	// Restore parent handle statement for nested handles
	CurrentHandleBB = ParentHandleBB;
	CurrentSafeBB = ParentSafeBB;

	// Restore parent error handler after generating the current handle and safe blocks
	CurrentErrorHandler = ParentErrorHandler;

	FLY_DEBUG_END("CodeGenModule", "GenStmtHandle");
}


void CodeGenModule::GenFailStmt(ASTFailStmt *FailStmt) {
	FLY_DEBUG_START("CodeGenModule", "GenStmtFail");

	// Store Fail value in ErrorHandler
	if (FailStmt->getFirstExpr() == nullptr) {
		CurrentErrorHandler->StoreInt(llvm::ConstantInt::get(CG.Int32Ty, 1));
	} else {
		// Store first expression in the error handler (this is the main error code)
		StoreFail(FailStmt->getFirstExpr(), CurrentErrorHandler);

		if (FailStmt->getSecondExpr()) {
			// If there is a second expression, we store the error code in the error handler
			StoreFail(FailStmt->getSecondExpr(), CurrentErrorHandler);

			if (FailStmt->getThirdExpr()) {
				// If there is a third expression, we store the error code in the error handler
				StoreFail(FailStmt->getThirdExpr(), CurrentErrorHandler);
			}
		}
	}

	if (CurrentHandleBB == nullptr) {
		// No enclosing handle, so we just return from the function
		Builder->CreateRetVoid();
	} else {
		Builder->CreateBr(CurrentSafeBB);
	}

	FLY_DEBUG_END("CodeGenModule", "GenStmtFail");
}

void CodeGenModule::StoreFail(ASTExpr *Expr, CodeGenError *CGE) {
	SemaExpr *SemaExpr = Expr->getSema();
	SemaExpr->accept(*this);
	llvm::Value *Value = SemaExpr->getCodeGen()->getValue();
	if (SemaExpr->getType()->isBool()) {
		// Bool is i1, need to zero extend to i32
		llvm::Value *Int32Value = Builder->CreateZExt(Value, CG.Int32Ty);
		CGE->StoreInt(Int32Value);
	} else if (SemaExpr->getType()->isInteger()) {
		SemaIntType *IntType = static_cast<SemaIntType *>(SemaExpr->getType());
		llvm::Value *Int32Value = Value;

		// Get the bit width from the integer kind
		unsigned BitWidth = static_cast<unsigned>(IntType->getIntKind());
		if (IntType->isSigned()) {
			BitWidth += 1; // Signed types have one less bit for the sign
		}

		if (BitWidth < 32) {
			// Smaller than i32, need to extend
			if (IntType->isSigned()) {
				Int32Value = Builder->CreateSExt(Value, CG.Int32Ty);
			} else {
				Int32Value = Builder->CreateZExt(Value, CG.Int32Ty);
			}
		} else if (BitWidth > 32) {
			// Larger than i32 (i64), need to truncate
			Int32Value = Builder->CreateTrunc(Value, CG.Int32Ty);
		}
		// If BitWidth == 32, no conversion needed

		CGE->StoreInt(Int32Value);
	} else if (SemaExpr->getType()->isString()) {
		CGE->StoreString(Value);
	} else if (SemaExpr->getType()->isClass()) {
		CGE->StoreObject(Value);
	} else if (SemaExpr->getType()->isEnum()) {
		CGE->StoreInt(Value);
	}
}


void CodeGenModule::GenIfStmt(ASTIfStmt *If) {
    FLY_DEBUG_START("CodeGenModule", "GenIfBlock");
    llvm::Function *Fn = CurrentFunction->getCodeGen()->getFunction();

    // If Block
	If->getExpr()->getSema()->accept(*this);
    llvm::Value *IfCond = If->getExpr()->getSema()->getCodeGen()->getValue();
    llvm::BasicBlock *IfBB = llvm::BasicBlock::Create(LLVMCtx, "ifthen", Fn);

    // Create End block
    llvm::BasicBlock *EndBB = llvm::BasicBlock::Create(LLVMCtx, "endif", Fn);

    if (!If->getElse()) {

        if (If->getElsif().empty()) { // If ...
            Builder->CreateCondBr(IfCond, IfBB, EndBB);
            Builder->SetInsertPoint(IfBB);
            GenStmt(If->getStmt());
            Builder->CreateBr(EndBB);
        } else { // If - elsif ...
            llvm::BasicBlock *ElsifBB = llvm::BasicBlock::Create(LLVMCtx, "elsif", Fn, EndBB);
            Builder->CreateCondBr(IfCond, IfBB, ElsifBB);

            // Start if-then
            Builder->SetInsertPoint(IfBB);
            GenStmt(If->getStmt());
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
            	Elsif->getExpr()->getSema()->accept(*this);
                llvm::Value *ElsifCond = Elsif->getExpr()->getSema()->getCodeGen()->getValue();
                Builder->CreateCondBr(ElsifCond, ElsifThenBB, NextElsifBB);

                Builder->SetInsertPoint(ElsifThenBB);
                GenStmt(Elsif->getStmt());
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
            GenStmt(If->getStmt());
            Builder->CreateBr(EndBB);
        } else { // If - Elsif - Else
            llvm::BasicBlock *ElsifBB = llvm::BasicBlock::Create(LLVMCtx, "elsif", Fn, ElseBB);
            Builder->CreateCondBr(IfCond, IfBB, ElsifBB);

            // Start if-then
            Builder->SetInsertPoint(IfBB);
            GenStmt(If->getStmt());
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
            	Elsif->getExpr()->getSema()->accept(*this);
                llvm::Value *ElsifCond = Elsif->getExpr()->getSema()->getCodeGen()->getValue();
                Builder->CreateCondBr(ElsifCond, ElsifThenBB, NextElsifBB);

                Builder->SetInsertPoint(ElsifThenBB);
                GenStmt(Elsif->getStmt());
                Builder->CreateBr(EndBB);

                ElsifBB = NextElsifBB;
            }
        }

        Builder->SetInsertPoint(ElseBB);
        GenStmt(If->getElse());
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
    	Elsif->getExpr()->getSema()->accept(*this);
        llvm::Value *Cond = Elsif->getExpr()->getSema()->getCodeGen()->getValue();
        llvm::BasicBlock *NextElsifBB = llvm::BasicBlock::Create(LLVMCtx, "elsif", Fn);
        Builder->CreateCondBr(Cond, ElsifBB, NextElsifBB);

        llvm::BasicBlock *ElsifThenBB = llvm::BasicBlock::Create(LLVMCtx, "elsifthen", Fn);
        Builder->SetInsertPoint(ElsifThenBB);
        GenStmt(Elsif->getStmt());
        GenElsifStmt(CGF, ElsifThenBB, It);
    }
}

void CodeGenModule::GenSwitchStmt(ASTSwitchStmt *Switch) {
    FLY_DEBUG_START("CodeGenModule", "GenSwitchBlock");
	llvm::Function *Fn = CurrentFunction->getCodeGen()->getFunction();

    // Create End Block
    llvm::BasicBlock *EndBB = llvm::BasicBlock::Create(LLVMCtx, "endswitch", Fn);

    // Push break target onto stack (switches don't have continue)
    BreakTargetStack.push_back(EndBB);

    // Create Expression evaluator for Switch
	Switch->getExpr()->getSema()->accept(*this);
    llvm::Value *SwitchVal = Switch->getExpr()->getSema()->getCodeGen()->getValue();
    llvm::SwitchInst *Inst = Builder->CreateSwitch(SwitchVal, EndBB);

    // Create Cases
    unsigned long Size = Switch->getCases().size();

    llvm::BasicBlock *NextCaseBB = nullptr;
    for (unsigned long i=0; i < Size; i++) {
        ASTRuleStmt *Case = Switch->getCases()[i];
    	Case->getExpr()->getSema()->accept(*this);
        llvm::Value *CaseVal = Case->getExpr()->getSema()->getCodeGen()->getValue();
        llvm::ConstantInt *CaseConst = llvm::cast<llvm::ConstantInt, llvm::Value>(CaseVal);
        llvm::BasicBlock *CaseBB = NextCaseBB == nullptr ?
                                   llvm::BasicBlock::Create(LLVMCtx, "case", Fn, EndBB) : NextCaseBB;
        Inst->addCase(CaseConst, CaseBB);
        Builder->SetInsertPoint(CaseBB);
        GenStmt(Case->getStmt());

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
        GenStmt(Switch->getDefault());
        Builder->CreateBr(EndBB);
    }

    // Pop break target from stack
    BreakTargetStack.pop_back();

    // Continue insertions into End Branch
    Builder->SetInsertPoint(EndBB);
}

void CodeGenModule::GenLoopStmt(ASTLoopStmt *Loop) {
    FLY_DEBUG_START("CodeGenModule", "GenLoopBlock");
	llvm::Function *Fn = CurrentFunction->getCodeGen()->getFunction();

    // Generate Init Statements
    for (ASTStmt *S : Loop->getInit()) {
    	GenStmt(S);
    }

    // Create Condition Block
    llvm::BasicBlock *CondBB = nullptr;
    if (Loop->getExpr()) {
        CondBB = llvm::BasicBlock::Create(LLVMCtx, "loopcond", Fn);
    }

    // Create Loop Block
    llvm::BasicBlock *LoopBB = LoopBB = llvm::BasicBlock::Create(LLVMCtx, "loop", Fn);

    // Create Post Block
    llvm::BasicBlock *PostBB = nullptr;
    if (!Loop->getPost().empty()) {
        PostBB = llvm::BasicBlock::Create(LLVMCtx, "looppost", Fn);
    }

    // Create End Block
    llvm::BasicBlock *EndBB = llvm::BasicBlock::Create(LLVMCtx, "loopend", Fn);

    // Push break and continue targets onto stacks
    BreakTargetStack.push_back(EndBB);
    // Continue target depends on whether we have a Post block or Condition block
    if (PostBB) {
        ContinueTargetStack.push_back(PostBB);
    } else if (CondBB) {
        ContinueTargetStack.push_back(CondBB);
    } else {
        ContinueTargetStack.push_back(LoopBB);
    }

    // Generate Code
    if (CondBB) {
        Builder->CreateBr(CondBB);

        // Create Condition
        Builder->SetInsertPoint(CondBB);
    	Loop->getExpr()->getSema()->accept(*this);
        llvm::Value *Cond = Loop->getExpr()->getSema()->getCodeGen()->getValue();
        Builder->CreateCondBr(Cond, LoopBB, EndBB);
    } else {
        Builder->CreateBr(LoopBB);
    }

    // Add to Loop
    Builder->SetInsertPoint(LoopBB);
    GenStmt(Loop->getLoop());
    if (PostBB) {
        Builder->CreateBr(PostBB);

        // Add to Post
        Builder->SetInsertPoint(PostBB);
    	for (ASTStmt *S : Loop->getPost()) {
    		GenStmt(S);
    	}
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

    // Pop break and continue targets from stacks
    BreakTargetStack.pop_back();
    ContinueTargetStack.pop_back();

    // Continue insertions into End Branch
    Builder->SetInsertPoint(EndBB);
}

void CodeGenModule::GenStmtLoopIn(ASTLoopInStmt *LoopIn) {
    FLY_DEBUG_START("CodeGenModule", "GenStmtLoopIn");

    // TODO: Implement proper for-in loop code generation
    // This requires:
    // 1. Determine if List is an array, string, or iterable collection
    // 2. Get the length/size of the collection
    // 3. Generate loop with iterator to access each element
    // 4. Assign each element to the Item variable
    // 5. Execute the loop body (Stmt)

    // For now, just generate the loop body as a placeholder
    GenStmt(LoopIn->getStmt());

    FLY_DEBUG_END("CodeGenModule", "GenStmtLoopIn");
}

std::string CodeGenModule::toIdentifier(llvm::StringRef Name, SemaNameSpace *NameSpace) {
	FLY_DEBUG_START("CodeGenModule", "toIdentifier");
	std::string Prefix = NameSpace ? std::string(NameSpace->getName()).append("_") : "";
	return Prefix.append(std::string(Name));
}