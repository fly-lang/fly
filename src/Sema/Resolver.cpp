//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/Resolver.cpp - The Resolver
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/Resolver.h"
#include "Sema/SemaResolverClass.h"
#include "Sema/Sema.h"
#include "Sema/SemaBuiltin.h"
#include "Sema/SemaImport.h"
#include "AST/ASTBuilder.h"
#include "Sema/SemaBuilder.h"
#include "Sema/SemaValidator.h"
#include "Basic/Logger.h"
#include "Sema/SymbolTable.h"
#include "Sema/SemaNameSpace.h"
#include "Sema/SemaModule.h"
#include "Sema/SemaClassType.h"
#include "Sema/SemaClassMethod.h"
#include "Sema/SemaClassAttribute.h"
#include "Sema/SemaEnumType.h"
#include "Sema/SemaMemberVar.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTClass.h"
#include "AST/ASTEnum.h"
#include "AST/ASTTypeRef.h"
#include "AST/ASTModule.h"
#include "AST/ASTArg.h"
#include "AST/ASTIfStmt.h"
#include "AST/ASTImport.h"
#include "AST/ASTFunction.h"
#include "AST/ASTCall.h"
#include "AST/ASTRef.h"
#include "AST/ASTVar.h"
#include "AST/ASTSwitchStmt.h"
#include "AST/ASTLoopStmt.h"
#include "AST/ASTLoopInStmt.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTValue.h"
#include "AST/ASTExpr.h"
#include "AST/ASTReturnStmt.h"
#include "AST/ASTHandleStmt.h"
#include "AST/ASTDeleteStmt.h"
#include "AST/ASTFailStmt.h"
#include "AST/ASTExprStmt.h"
#include "AST/ASTOpExpr.h"
#include "CodeGen/CodeGen.h"
#include "Basic/Diagnostic.h"
#include "Basic/Debug.h"
#include "llvm/ADT/StringMap.h"
#include <AST/ASTComment.h>
#include <llvm/Transforms/IPO/FunctionImport.h>
#include <Sema/SemaCall.h>
#include <Sema/SemaComment.h>
#include <Sema/SemaEnumEntry.h>
#include <Sema/SemaFunction.h>
#include <Sema/SemaLocalVar.h>
#include <Sema/Registry.h>
#include "Basic/Diagnostic.h"

#include <AST/ASTAlias.h>
#include <Sema/SemaValue.h>

using namespace fly;

Resolver::Resolver(DiagnosticsEngine &Diags, Registry &Reg) : Diags(Diags),
	CurrentModule(nullptr),
	BuiltinScope(CreateBuiltinScope()),
	Reg(Reg),
	CurrentNameSpace(Reg.getDefaultNameSpace()),
    CurrentScope(Reg.getDefaultNameSpace()->getSymbols()) {

}

SymbolTable* Resolver::CreateBuiltinScope() {
	SymbolTable* Builtin = new SymbolTable(nullptr);

	auto BoolType = SemaBuiltin::getBoolType();
	auto ByteType = SemaBuiltin::getByteType();
	auto UShortType = SemaBuiltin::getUShortType();
	auto ShortType = SemaBuiltin::getShortType();
	auto UIntType = SemaBuiltin::getUIntType();
	auto IntType = SemaBuiltin::getIntType();
	auto ULongType = SemaBuiltin::getULongType();
	auto LongType = SemaBuiltin::getLongType();
	auto FloatType = SemaBuiltin::getFloatType();
	auto DoubleType = SemaBuiltin::getDoubleType();
	auto StringType = SemaBuiltin::getStringType();
	auto VoidType = SemaBuiltin::getVoidType();
	auto ErrorType = SemaBuiltin::getErrorType();

	// Insert Builtin Types
	Builtin->insert(new Symbol(BoolType->getName(), SemaKind::TYPE, BoolType));
	Builtin->insert(new Symbol(ByteType->getName(), SemaKind::TYPE, ByteType));
	Builtin->insert(new Symbol(UShortType->getName(), SemaKind::TYPE, UShortType));
	Builtin->insert(new Symbol(ShortType->getName(), SemaKind::TYPE, ShortType));
	Builtin->insert(new Symbol(UIntType->getName(), SemaKind::TYPE, UIntType));
	Builtin->insert(new Symbol(IntType->getName(), SemaKind::TYPE, IntType));
	Builtin->insert(new Symbol(ULongType->getName(), SemaKind::TYPE, ULongType));
	Builtin->insert(new Symbol(LongType->getName(), SemaKind::TYPE, LongType));
	Builtin->insert(new Symbol(FloatType->getName(), SemaKind::TYPE, FloatType));
	Builtin->insert(new Symbol(DoubleType->getName(), SemaKind::TYPE, DoubleType));
	Builtin->insert(new Symbol(StringType->getName(), SemaKind::TYPE, StringType));
	Builtin->insert(new Symbol(VoidType->getName(), SemaKind::TYPE, VoidType));
	Builtin->insert(new Symbol(ErrorType->getName(), SemaKind::TYPE, ErrorType));

	return Builtin;
}

/**
 * Write Diagnostics
 * @param Loc
 * @param DiagID
 * @return
 */
DiagnosticBuilder Resolver::Diag(const SourceLocation &Loc, unsigned DiagID) const {
	return Diags.Report(Loc, DiagID);
}

DiagnosticBuilder Resolver::Diag(unsigned DiagID) const {
	return Diags.Report(DiagID);
}

void Resolver::visit(ASTModule &AST) {
	CurrentModule = new SemaModule(AST);
	Reg.addModule(CurrentModule);

	for (size_t i = 0; i < AST.getDefinitions().size(); ++i) {
		auto Def = AST.getDefinitions()[i];

		if (i == 0) {
			if (Def->getKind() == ASTKind::AST_NAMESPACE) {
				// Visit Definition
				Def->accept(*this);
			} else {
				// Set the Module NameSpace
				CurrentNameSpace = Reg.getDefaultNameSpace();
				CurrentScope = CurrentNameSpace->getSymbols();
			}
		} else {
			if (Def->getKind() == ASTKind::AST_NAMESPACE) {
				// Error: Namespace must be the first definition in the Module
				Diag(AST.getLocation(), diag::err_syntax_error);

				// Cannot go ahead with resolving
				return;
			}

			// Enter Module Scope
			EnterScope();

			// Visit Definition
			Def->accept(*this);
		}
	}

	ExitScope();
}

void Resolver::visit(ASTNameSpace &AST) {
	// Build the CurrentNameSpace
	std::string FQName = "";
	SemaNameSpace *NameSpace = nullptr;
	for (auto It = AST.getNames().begin(); It != AST.getNames().end(); ++It) {
		// Generate the full name
		FQName += (It == AST.getNames().begin()) ? std::string(*It) : "." + std::string(*It);

		// Add as Parent the previous NameSpace
		NameSpace = Reg.getOrAddFQNameSpace(FQName, NameSpace);
	}

	CurrentModule->NameSpace = NameSpace;
	CurrentNameSpace = NameSpace;
	CurrentScope = CurrentNameSpace->getSymbols();;
}

void Resolver::visit(ASTImport &AST) {
	// Error: Empty Import
	if (AST.getName().empty()) {
		Diag(AST.getLocation(), diag::err_sema_import_undefined);
	}

	// TODO
	// Error: name is equals to the current ASTModule namespace
	// if (AST.getName() == Module->getNameSpace()->getName()) {
	// 	Diag(AST.getLocation(), diag::err_import_conflict_namespace) << AST.getName();
	// }

	// Replace with alias name if exists
	llvm::StringRef Name = AST.getName();

	// Error: alias is equals to the current ASTModule namespace
	// if (AST.getAlias()) {
	//
	// 	// Set Import Name
	// 	Name = AST.getAlias()->getName();
	//
	// 	// Check Alias
	// 	if (Module->getImports().lookup(Name) != nullptr) {
	// 		Diag(AST.getLocation(), diag::err_conflict_import_alias) << Name;
	// 		return;
	// 	}
	//
	// 	if (AST.getAlias()->getName() == Module->getNameSpace()->getName()) {
	// 		Diag(AST.getAlias()->getLocation(), diag::err_alias_conflict_namespace) << AST.getAlias()->getName();
	// 		return;
	// 	}
	// }

	// Search Namespace in Symbol Table
	SemaImport *Import = new SemaImport(AST);

	// Add Import to the Module Imports for next symbols resolution
	CurrentModule->Imports.push_back(Import);
}

void Resolver::visit(ASTFunction &AST) {
	auto* Func = new SemaFunction(AST);

	// Add to Module
	CurrentModule->Nodes.push_back(Func);

	// Add to Symbol Table
	Symbol *Sym = new Symbol(AST.getName(), SemaKind::FUNCTION, Func);
	CurrentScope->insert(Sym);

	// Enter Function Scope
	EnterScope();
	// Add parameters, body ...
	ExitScope();
}

void Resolver::visit(ASTClass &AST) {
	auto* Class = new SemaClassType(AST);

	// Add to Module
	CurrentModule->Nodes.push_back(Class);

	// Add to Symbol Table
	Symbol *Sym = new Symbol(AST.getName(), SemaKind::CLASS, Class);
	CurrentScope->insert(Sym);

	// Enter Class Scope
	EnterScope();
	for (auto Def : AST.getDefinitions())
		Def->accept(*this);
	ExitScope();
}

void Resolver::visit(ASTEnum &AST) {
	auto* Enum = new SemaEnumType(AST);

	// Add to Module
	CurrentModule->Nodes.push_back(Enum);

	// Add to Symbol Table
	Symbol *Sym = new Symbol(AST.getName(), SemaKind::ENUM, Enum);
	CurrentScope->insert(Sym);

	// Enter Enum Scope
	EnterScope();
	for (auto Def : AST.getDefinitions())
		Def->accept(*this);
	ExitScope();
}

void Resolver::Resolver::EnterScope() {
	SymbolTable* NewScope = new SymbolTable(CurrentScope);
	CurrentScope = NewScope;
}

void Resolver::Resolver::ExitScope() {
	SymbolTable* Old = CurrentScope;
	CurrentScope = CurrentScope->getParent();
	delete Old; // opzionale, se vuoi deallocare
}

void Resolver::Resolve() {
	for (auto Module : Reg.getModules()) {
		ResolveImports(Module);

		// Resolve Functions
		ResolveFunctions(Module);

		// Resolve Types
		ResolveTypes(Module);
	}

	// Resolve Bodies
	for (auto Body : Reg.getBodies()) {
		ResolveStmtBlock(Body);
	}
}

// void Resolver::ResolveComment(SemaComment *Comment, ASTNode* AST) {
// 	if (AST->getKind() == ASTKind::AST_FUNCTION) {
// 		ASTFunction * Function = (ASTFunction *) AST;
// 		SemaValidator::CheckCommentParams(Comment, Function->getParams());
// 		SemaValidator::CheckCommentReturn(Comment, Function->getReturnTypeRef());
// 		SemaValidator::CheckCommentFail(Comment);
// 	}
// }

/**
 * Resolve Module GlobalVar Definitions
 */
// TODO: remove GlobalVar
// void Resolver::ResolveGlobalVars() {
// 	for (auto &Entry : Module->getGlobalVars()) {
// 		SemaGlobalVar *Sema = Entry.getValue();
// 		ASTVar *AST = Sema->getAST();
//
// 		if (Sema->Comment) {
// 			ResolveComment(Sema->Comment, AST);
// 		}
//
// 		// Check Expr Value
// 		if (AST->Expr && AST->Expr->getExprKind() != ASTExprKind::EXPR_VALUE) {
// 			Diag(AST->Expr->getLocation(), diag::err_invalid_gvar_value);
// 		}
//
// 		// Resolve Type
// 		if (ResolveTypeRef(AST->TypeRef)) {
// 			Sema->Type = AST->TypeRef->getSema();
// 		}
// 	}
// }


void Resolver::ResolveImports(SemaModule *Module) {
}

/**
 * Resolve Module Function Definitions
 */
void Resolver::ResolveFunctions(SemaModule *Module) {
	for (auto &Entry : Module->getFunctions()) {
		SemaFunction *Sema = Entry.getValue();
		ASTFunction *AST = Sema->getAST();

		if (Sema->Comment) {
			ResolveComment(Sema->Comment, AST);
		}

		// Resolve Return Type
		if (ResolveTypeRef(AST->ReturnTypeRef)) {
			Sema->ReturnType = AST->ReturnTypeRef->getSema();
		}

		// Resolve Parameters Types
		for (auto Param : AST->getParams()) {
			// Check duplicated params
			// TODO
			//SemaValidator::CheckDuplicateParams(Function->Params, Param);

			// resolve parameter type
			if (ResolveTypeRef(Param->TypeRef)) {
				SemaParam *P = SemaBuilder::CreateParam(Param);
				P->Type = Param->TypeRef->getSema();
                Sema->Params.push_back(P);
            }
		}

		// Add to Body list for resolve in the next step
		Bodies.push_back(AST->Body);
	}
}

/**
 * Resolve Module Identity Definitions
 */
void Resolver::ResolveTypes(SemaModule *Module) {
	for (auto &TypeEntry : Module->getTypes()) {
		SemaType *Sema = TypeEntry.getValue();

		if (Sema->isClass()) {
			SemaClassType * ClassType = static_cast<SemaClassType *>(Sema);

			// Resolve Comment
			if (ClassType->Comment) {
				ResolveComment(ClassType->Comment, ClassType->getAST());
			}

			// Create Class Type Resolver
			SemaResolverClass *ResolverClass = new SemaResolverClass(this, ClassType);
			this->ResolverClasses.push_back(ResolverClass);

		} else if (Sema->isEnum()) {
			SemaEnumType * EnumType = static_cast<SemaEnumType *>(Sema);

			// Resolve Comment
			if (EnumType->Comment) {
				ResolveComment(EnumType->Comment, EnumType->getAST());
			}

			// Resolve Enum Type
			ResolveEnumType(EnumType);
		}

		CurrentNameSpace->Types.insert(std::make_pair(Sema->getName(), Sema));
	}
}

void Resolver::ResolveClassTypes() {
	for (SemaResolverClass *ResolverClass : ResolverClasses) {
		ResolverClass->Resolve();
	}
}

void Resolver::ResolveEnumType(SemaEnumType *Sema) {
	if (Sema->Comment) {
		ResolveComment(Sema->Comment, Sema->getAST());
	}

	SemaComment *Comment = nullptr;
	for (auto &AST: Sema->getAST()->getDefinitions()) {
		switch (AST->getKind()) {

			// Resolve Enum Var: Enum Entry
			case ASTKind::AST_VAR: {
				SemaBuilder::CreateEnumEntry(Sema, static_cast<ASTVar *>(AST), Comment);
				Comment = nullptr;
			}	break;

			// Resolve Enum Entry Comment
			case ASTKind::AST_COMMENT:
				Comment = SemaBuilder::CreateComment(static_cast<ASTComment *>(AST));
			break;

			default:
				// Error: invalid declaration in class
					Diag(AST->getLocation(), diag::err_syntax_error);
			break;
		}
	}
}

bool Resolver::ResolveStmt(ASTStmt *Stmt) {
    switch (Stmt->getStmtKind()) {

        case ASTStmtKind::STMT_BLOCK:
            return ResolveStmtBlock(static_cast<ASTBlockStmt *>(Stmt));
		case ASTStmtKind::STMT_RULE: {
			ASTRuleStmt *Rule = static_cast<ASTRuleStmt *>(Stmt);
			return ResolveExpr(Rule->getParent(), Rule->getRule()) && ResolveStmt(Rule->getStmt());
		}
        case ASTStmtKind::STMT_IF:
            return ResolveStmtIf(static_cast<ASTIfStmt *>(Stmt));
        case ASTStmtKind::STMT_SWITCH:
            return ResolveStmtSwitch(static_cast<ASTSwitchStmt *>(Stmt));
        case ASTStmtKind::STMT_LOOP:
            return ResolveStmtLoop(static_cast<ASTLoopStmt *>(Stmt));
        case ASTStmtKind::STMT_LOOP_IN:
            return ResolveStmtLoopIn(static_cast<ASTLoopInStmt *>(Stmt));
        case ASTStmtKind::STMT_VAR:
            return ResolveStmtVar(static_cast<ASTVarStmt *>(Stmt));
        case ASTStmtKind::STMT_EXPR:
            return ResolveExpr(Stmt->Parent, static_cast<ASTExprStmt *>(Stmt)->Expr);
        case ASTStmtKind::STMT_FAIL:
            return ResolveStmtFail(static_cast<ASTFailStmt *>(Stmt));
        case ASTStmtKind::STMT_HANDLE:
            return ResolveStmtHandle(static_cast<ASTHandleStmt *>(Stmt));
        case ASTStmtKind::STMT_DELETE:
            return ResolveRef(Stmt->Parent, static_cast<ASTDeleteStmt *>(Stmt)->VarRef);
        case ASTStmtKind::STMT_RETURN: {
        	ASTReturnStmt *ReturnStmt = static_cast<ASTReturnStmt *>(Stmt);
        	ASTTypeRef * ReturnType = ReturnStmt->Parent->getFunction()->getReturnTypeRef(); // Force Return Expr to be of Return Type
			bool Success = true;
        	if (ReturnStmt->Expr != nullptr) {
            	if (ResolveExpr(ReturnStmt->Parent, ReturnStmt->Expr)) {
            		//SemaValidator::CheckConvertibleTypes(, ReturnType->getSym());
            		ReturnStmt->Expr->Type = ReturnType->getSema();
            	}
            } else {
            	if (!ReturnStmt->Parent->getFunction()->getReturnTypeRef()->getSema()->isVoid()) {
            		Diag(ReturnStmt->getLocation(), diag::err_invalid_return_type);
            	}
            }
            return Success;
        }
        case ASTStmtKind::STMT_BREAK:
        case ASTStmtKind::STMT_CONTINUE:
            return true;
    }

    assert(false && "Invalid ASTStmtKind");
}

bool Resolver::ResolveStmtBlock(ASTBlockStmt *Block) {
    // Resolve LocalVar Type
    for (auto &VarEntry : Block->LocalVars) {
    	ASTVar * LocalVar = VarEntry.getValue();

    	// Resolve LocalVar Type
        ResolveTypeRef(LocalVar->TypeRef);

    	// Create LocalVar Sema
    	SemaLocalVar * Sema = SemaBuilder::CreateLocalVar(LocalVar);

    	// Assign the Type Symbol to LocalVar
    	if (LocalVar->getTypeRef() != nullptr && LocalVar->getTypeRef()->isResolved()) {
    		Sema->Type = LocalVar->getTypeRef()->getSema();
    	}

    	// Add LocalVar to the Function Base LocalVars
    	Block->getFunction()->getSema()->LocalVars.push_back(LocalVar->getSema());
    }

    // Resolve Statements
    for (ASTStmt *Stmt : Block->Content) {
        ResolveStmt(Stmt);
    }

    // Check LocalVar initialization
    // TODO
//    for (auto &LocalVar : Block->LocalVars) {
//        if (!LocalVar.second->isInitialized())
//            Diag(LocalVar.getValue()->getLocation(), diag::err_sema_uninit_var) << LocalVar.getValue()->getName();
//    }

    return true;
}

bool Resolver::ResolveStmtIf(ASTIfStmt *IfStmt) {
    bool Success = ResolveExpr(IfStmt->getParent(), IfStmt->Rule);
	IfStmt->Rule->Type = SemaBuiltin::getBoolType();

    Success &= ResolveStmt(IfStmt->Stmt);
    for (ASTRuleStmt *Elsif : IfStmt->Elsif) {
        Success &= ResolveExpr(IfStmt->getParent(), Elsif->Rule);
    	Elsif->Rule->Type = SemaBuiltin::getBoolType();
        Success &= ResolveStmt(Elsif->Stmt);
    }
    if (Success && IfStmt->Else) {
        Success = ResolveStmt(IfStmt->Else);
    }
    return Success;
}

bool Resolver::ResolveStmtSwitch(ASTSwitchStmt *SwitchStmt) {
    assert(SwitchStmt && "Switch Block cannot be null");

    bool Success = ResolveRef(SwitchStmt->getParent(), SwitchStmt->Ref) &&
    	SwitchStmt->getRef()->getSema()->getType()->isInteger();
    for (ASTRuleStmt *Case : SwitchStmt->Cases) {
    	Success &= ResolveExpr(SwitchStmt, Case->getRule());
        Success &= Case->getRule()->getType()->isInteger() && ResolveStmt(Case);
    }
    return Success && ResolveStmt(SwitchStmt->Default);
}

bool Resolver::ResolveStmtLoop(ASTLoopStmt *LoopStmt) {
    // Check Loop is not null or empty
    if (LoopStmt->Stmt == nullptr) {
    	Diag(diag::err_sema_generic);
    	return false;
    }

	if (LoopStmt->getRule() == nullptr) { // Error: empty condition expr
		Diag(diag::err_parse_empty_while_expr);
		return false;
	}

	bool Success = true;
    // Check Init
    if (LoopStmt->Init) {
        LoopStmt->Stmt->Parent = LoopStmt->Init;
        Success = ResolveStmt(LoopStmt->Init);
        Success &= ResolveExpr(LoopStmt->Init, LoopStmt->Rule);
    	LoopStmt->Rule->Type = SemaBuiltin::getBoolType();
    } else {
        Success = ResolveExpr(LoopStmt->Parent, LoopStmt->Rule);
    	LoopStmt->Rule->Type = SemaBuiltin::getBoolType();
    }
    Success = SemaValidator::CheckConvertibleTypes(LoopStmt->getRule()->getType(), SemaBuiltin::getBoolType());
    Success &= ResolveStmt(LoopStmt->Stmt);
    Success &= LoopStmt->Post ? ResolveStmt(LoopStmt->Post) : true;
    return Success;
}

bool Resolver::ResolveStmtLoopIn(ASTLoopInStmt *LoopInStmt) {
    return ResolveRef(LoopInStmt->Parent, LoopInStmt->VarRef) && ResolveStmtBlock(LoopInStmt->Block);
}

bool Resolver::ResolveStmtVar(ASTVarStmt *VarStmt) {
    if (ResolveRef(VarStmt->Parent, VarStmt->VarRef)) {
	    SemaVar *Var = static_cast<SemaVar *>(VarStmt->getVarRef()->getSema());
    	if (Var && VarStmt->getExpr() != nullptr) {
    		if (ResolveExpr(VarStmt->Parent, VarStmt->Expr)) {
    			VarStmt->Expr->Type = Var->getType();
    			return true;
    		}
    	}
    }
	return false;
}

bool Resolver::ResolveStmtFail(ASTFailStmt *FailStmt) {
	// Resolve Fail Expr
	if (FailStmt->Expr)
		return ResolveExpr(FailStmt->Parent, FailStmt->Expr);

    return true;
}

bool Resolver::ResolveStmtHandle(ASTHandleStmt *HandleStmt) {
    if (HandleStmt->ErrorHandlerRef)
        ResolveRef(HandleStmt->getParent(), HandleStmt->ErrorHandlerRef);
    return ResolveStmt(HandleStmt->Handle);
}

bool Resolver::ResolveValue(ASTValue *AST) {
	assert(AST && "Value cannot be null");
	switch (AST->getTypeKind()) {

		// Bool Value
		case ASTValueKind::VAL_BOOL:
			return SemaBuilder::CreateBoolValue(static_cast<ASTBoolValue *>(AST));

		// Number Value
		case ASTValueKind::VAL_NUMBER:
			return SemaBuilder::CreateNumberValue(static_cast<ASTNumberValue *>(AST));

		// String Value
		case ASTValueKind::VAL_STRING:
			return SemaBuilder::CreateStringValue(static_cast<ASTStringValue *>(AST));

		// Array Value
		case ASTValueKind::VAL_ARRAY: {
			ASTArrayValue * ArrayAST = static_cast<ASTArrayValue *>(AST);
			SemaArrayValue *Array = SemaBuilder::CreateArrayValue(ArrayAST);
			for (auto Value : ArrayAST->getValues()) {
				if (ResolveValue(Value))
					Array->Values.push_back(Value->getSema());
			}
			return Array;
		}

		// Struct Value
		case ASTValueKind::VAL_STRUCT: {
			ASTStructValue * StructAST = static_cast<ASTStructValue *>(AST);
			SemaStructValue *Struct = SemaBuilder::CreateStructValue(StructAST);
			for (auto &Entry : StructAST->getValues()) {
				if (ResolveValue(Entry.second))
                    Struct->Values.insert(std::make_pair(Entry.getKey(), Entry.second->getSema()));
			}
			return Struct;
		}

		case ASTValueKind::VAL_NULL:
			assert(AST && "Unexpected null value");
			break;
	}

    assert(false && "Invalid ASTValueKind");
	return false;
}

/**
 * ResolveModule Expr contents
 * @param Expr
 * @return true if no error occurs, otherwise false
 */
bool Resolver::ResolveExpr(ASTStmt *Stmt, ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveExpr", Logger()
            .Attr("Expr", Expr)
            .Attr("Type", Expr).End());

    switch (Expr->getExprKind()) {

    	// Value Expr
        case ASTExprKind::EXPR_VALUE: {
            ASTValue *Value = static_cast<ASTValueExpr*>(Expr)->getValue();

        	// Select the best Value between: bool, number, string, array, struct
        	if (ResolveValue(Value)) {
        		Expr->Type = Value->getSema()->getType();
        		return true;
        	}
        } break;

    	// VarRef Expr
        case ASTExprKind::EXPR_VAR_REF: {
            ASTRef *VarRef = static_cast<ASTVarRefExpr*>(Expr)->getVarRef();

        	// Start to resolve the VarRef from root
            if (ResolveRef(Stmt, VarRef)) {

            	// Validate Call
            	SemaValidator::CheckVar(Stmt, VarRef);

                Expr->Type = VarRef->getSema()->getType();
                return true;
            }
        } break;

    	// Call Expr
        case ASTExprKind::EXPR_CALL: {
            ASTCall *Call = static_cast<ASTCallExpr*>(Expr)->getCall();

        	// Start to resolve the CallRef from root
            if (ResolveRef(Stmt, Call)) {

            	// Validate Call
            	SemaValidator::CheckCall(Stmt, Call);

                switch (Call->getCallKind()) {

                	// Call a Function
                    case ASTCallKind::CALL_DIRECT:
                        Expr->Type = Call->getSema()->getFunction()->getReturnType();
                        return true;

                	// Call a Constructor Method
                    case ASTCallKind::CALL_NEW: {
                    	SemaClassMethod *Method = static_cast<SemaClassMethod *>(Call->getSema()->getFunction());
                        Expr->Type = Method->getClass();
                    	return true;
                    }
                }
            }
        } break;

    	// Operator Expr
        case ASTExprKind::EXPR_OP: {

            switch (static_cast<ASTOpExpr*>(Expr)->getOpExprKind()) {

                case ASTOpExprKind::OP_UNARY: {
                    ASTUnaryOpExpr *Unary = static_cast<ASTUnaryOpExpr*>(Expr);
                    if (ResolveExpr(Stmt, const_cast<ASTExpr*>(Unary->Expr))) {
	                    Expr->Type = Unary->getExpr()->getType();
                    	return true;
                    }
                }

                case ASTOpExprKind::OP_BINARY: {
                    ASTBinaryOpExpr *Binary = static_cast<ASTBinaryOpExpr*>(Expr);

                    if (ResolveExpr(Stmt, Binary->LeftExpr) && ResolveExpr(Stmt, Binary->RightExpr)) {

                    	// Check if Left and Right Expr are resolved
                    	SemaType * LeftType = Binary->getLeftExpr()->getType();
                    	SemaType * RightType = Binary->getRightExpr()->getType();

                        if (Binary->getTypeKind() == ASTBinaryOpTypeExprKind::OP_BINARY_ARITH ||
                                Binary->getTypeKind() == ASTBinaryOpTypeExprKind::OP_BINARY_COMPARISON) {

                        	// Check Compatible Types Bool/Bool, Float/Float, Integer/Integer
                        	if (SemaValidator::CheckArithTypes(Binary->getLeftExpr()->getType(),
																  Binary->getRightExpr()->getType())) {
                            	// Set respectively the Left or Right Expr Type by chose the Expr which is not a Value Type
                            	// Ex.
                            	// int a = 0
                            	// int b = a + 1
                            	// 1 will have type int
                            	if (Binary->getLeftExpr()->getExprKind() == ASTExprKind::EXPR_VALUE &&
									Binary->getRightExpr()->getExprKind() != ASTExprKind::EXPR_VALUE) {
                            		Binary->LeftExpr->Type = RightType;
								} else if (Binary->getRightExpr()->getExprKind() == ASTExprKind::EXPR_VALUE &&
                            		Binary->getLeftExpr()->getExprKind() != ASTExprKind::EXPR_VALUE) {
                            		Binary->RightExpr->Type = LeftType;
                            	}

                                // Promotes First or Second Expr Types in order to be equal
                                if (LeftType->isInteger()) {
                                    if (static_cast<SemaIntType *>(LeftType)->getIntKind() >
                                    	static_cast<SemaIntType*>(RightType)->getIntKind())
                                        Binary->Type = LeftType;
                                    else
                                        Binary->Type = RightType;
                                } else if (LeftType->isFloatingPoint()) {
                                    if (static_cast<SemaFloatType*>(LeftType)->getFPKind() >
                                    	static_cast<SemaFloatType*>(RightType)->getFPKind())
                                		Binary->Type = LeftType;
                                    else
                                		Binary->Type = RightType;
                                }

                                Binary->Type = Binary->getTypeKind() == ASTBinaryOpTypeExprKind::OP_BINARY_ARITH ?
                                               LeftType : SemaBuiltin::getBoolType();
                        		return true;
                            } else {
                            	Diag(Binary->getLocation(), diag::err_sema_types_operation)
											<< LeftType->getName()
											<< RightType->getName();
                            }
                        } else if (Binary->getTypeKind() == ASTBinaryOpTypeExprKind::OP_BINARY_LOGIC) {
                        	if (SemaValidator::CheckLogicalTypes(LeftType, RightType)) {
                        		Binary->Type = SemaBuiltin::getBoolType();
                        		return true;
                        	} else {
                        		Diag(Binary->getLocation(), diag::err_sema_types_logical)
									<< LeftType->getName()
									<< RightType->getName();
                        	}
                        }
                    }
                } break;

                case ASTOpExprKind::OP_TERNARY: {
                    ASTTernaryOpExpr *Ternary = static_cast<ASTTernaryOpExpr*>(Expr);
                    if (ResolveExpr(Stmt, Ternary->ConditionExpr) &&
                              SemaValidator::CheckConvertibleTypes(Ternary->getConditionExpr()->getType(), SemaBuiltin::getBoolType()) &&
                              ResolveExpr(Stmt, Ternary->TrueExpr) &&
                              ResolveExpr(Stmt, Ternary->FalseExpr)) {
	                    Ternary->Type = Ternary->getTrueExpr()->getType(); // The group type is equals to the second type
                    	return true;
                    }
                }
            } break;
        }
        default:
            assert(0 && "Invalid ASTExprKind");
    }

    return false;
}

bool Resolver::ResolveTypeRef(ASTTypeRef *&TypeRef) {
	if (!TypeRef->Resolved) {

		// Set current with the Top Parent
		ASTRef *Current = TypeRef;
		while (Current->getParent() != nullptr) {
			Current->getParent()->Child = Current;
			Current = Current->getParent();
		}

		// Ref is a CurrentNameSpace ?
		SemaNameSpace * CurrentNameSpace = ResolveNameSpace(Current);

		// Resolve from top-bottom
		if (TypeRef->Sema) {
			// TypeRef is already resolved
			TypeRef->Resolved = true;
			return true;
		}

		// TypeRef is an Array
		if (TypeRef->isArray()) {
			auto ArrayTypeRef = static_cast<ASTArrayTypeRef *>(TypeRef);
			return ResolveTypeRef(ArrayTypeRef->TypeRef);
		}

		// Type is Class or Enum
		TypeRef->Sema = ResolveType(TypeRef->getName(), CurrentNameSpace);

		// Take Identity from CurrentNameSpace
		TypeRef->Resolved = TypeRef->Sema != nullptr; // Evict Cycle Loop: can be resolved only now
	}

	if (!TypeRef->Sema) {
		Diag(TypeRef->getLocation(), diag::err_unref_type);
		return false;
	}

	return true;
}

/**
 * Resolve a Reference, continue to resolve until the Ref is completely resolved
 * @param Stmt
 * @param Ref
 * @param NameSpaces
 * @param ...
 * @return
 */
void Resolver:: ResolveFromTopRef(ASTStmt *Stmt, ASTRef *Ref, SemaNameSpace *CurrentNameSpace) {
	if (!Ref->Resolved) {

		// Ref is a Function
		if (Ref->isCall()) {
			ASTCall *CallRef = static_cast<ASTCall *>(Ref);
			SemaCall *Call = ResolveCall(Stmt, CallRef, CurrentNameSpace);

			if (CallRef->Child)
				ResolveInstanceRef(Stmt, CallRef->Child, Call);
		}

		//Ref is a Var
		else if (Ref->isVarRef()) {
			SemaVar *Sema = ResolveVar(Stmt, Ref);

			if (Ref->Child)
				ResolveInstanceRef(Stmt, Ref->Child, Sema);
		}

		// Ref is a Class or an Enum Type?
		else {
			SemaType *Type = ResolveType(Ref->getName(), CurrentNameSpace);

			// Call can be a local or base class constructor method
			if (Ref->Child) {

				// Check if Type is a Current or Base Class of the current Method
				// class TestClass : BaseClass, BaseClass2 {
				//   void do() {
				//		BaseClass.do()
				//      or
				//      TestClass.do()
				//   }
				// }
				if (Type->isClass() && Stmt->getFunction()->getSema()->getKind() == SemaFunctionKind::CLASS_METHOD &&
					static_cast<SemaClassType *>(Type)->isBaseOrEquals(static_cast<SemaClassMethod *>(Stmt->getFunction()->getSema())->getClass())) {

					// Get the Class Instance related to Class Type
					// TestClass.do()  TestClass->this  TestClass.do(TestClass->this)
					// BaseClass.do()  BaseClass->this
					SemaClassInstance *This = static_cast<SemaClassMethod *>(Stmt->getFunction()->getSema())->getClass()->getThis();

					// Resolve instance as Class Instance of the current class or base class
					ResolveInstanceRef(Stmt, Ref->Child, This);

				} else {

					// Resolve a Static Ref to a Class or Enum
					ResolveStaticRef(Stmt, Ref->Child, Type);
				}
			}
		}
	}
}

/**
 * Resolve static Ref to a Class or Enum
 * @param Type
 * @param Ref
 * @return
 */
void Resolver:: ResolveStaticRef(ASTStmt *Stmt, ASTRef *Ref, SemaType *Type) {
	if (!Ref->Resolved) {

		// Class
		if (Type->isClass()) {
			SemaClassType *ClassType = static_cast<SemaClassType *>(Type);

			// class method
			if (Ref->isCall()) {
				ASTCall *Call = static_cast<ASTCall *>(Ref);

				// Set as Resolved
				Call->Resolved = true;

				SmallVector<SemaType *, 8> CallTypes = ResolveCallArgTypes(Stmt, Call);
				std::string Mangled = SemaFunctionBase::MangleFunction(Call->getName(), CallTypes);
				SemaClassMethod* Method = ClassType->getMethods().lookup(Mangled);

				// Create a call to class method
				if (Method) {

					// Check if Method belongs to a super class or is static
					if (!ClassType->isDerivedOrEquals(Method->getClass()) && !Method->isStatic()) {
                    	// Error: method cannot be called statically
                    	Diag(Ref->getLocation(), diag::err_syntax_error) << Ref->getName();
                    }

					Call->Sema = SemaBuilder::CreateCall(Call);

					// Set the Call Sema ErrorHandler
					ResolveErrorHandler(Stmt, Call->Sema);

					// Set the Call Sema Function
					Call->Sema->Function = Method;

					// Parent is not set, because this is a static call
					Call->Sema->setParent(nullptr);

					if (Ref->Child)
						ResolveInstanceRef(Stmt, Ref->Child, Call->Sema);
				}
			}

			// class attribute
			else {

				// Set as Resolved
				Ref->Resolved = true;

				SemaClassAttribute *Attr = ClassType->getAttributes().lookup(Ref->getName());
				if (Attr) {

					if (!Attr->isStatic()) {
						// Error: cannot resolve a non-static attribute without a parent
						Diag(Ref->getLocation(), diag::err_syntax_error) << Ref->getName();
					}

					// Resolve a static attribute
					Ref->Sema = Attr;

					// Parent is not set, because this is a static attribute
					Ref->Sema->setParent(nullptr);

					if (Ref->Child)
						ResolveInstanceRef(Stmt, Ref->Child, Ref->Sema);
				}
			}
		}

		// Enum
		else if (Type->isEnum()) {
			ResoveEnumRef(Stmt, Ref, static_cast<SemaEnumType *>(Type));
		}
	}
}

/**
 * Resolve static Ref to a Class or Enum
 * @param Type
 * @param Ref
 * @return
 */
void Resolver:: ResolveInstanceRef(ASTStmt *Stmt, ASTRef *Ref, SemaResult *Parent) {
	if (!Ref->Resolved) {

		// Class
		if (Parent->getType()->isClass()) {
			SemaClassType *ClassType = static_cast<SemaClassType *>(Parent->getType());

			// class method
			if (Ref->isCall()) {
				ASTCall *Call = static_cast<ASTCall *>(Ref);

				// Set as Resolved
				Call->Resolved = true;

				SmallVector<SemaType *, 8> CallTypes = ResolveCallArgTypes(Stmt, Call);
				std::string Mangled = SemaFunctionBase::MangleFunction(Call->getName(), CallTypes);

				// Search method in the class instance
				SemaClassMethod* CalledMethod = ClassType->getMethods().lookup(Mangled);

				// Search method in the class instance base classes
				if (!CalledMethod) {
					for (SemaClassType *BaseClass : ClassType->getBaseClasses()) {
						while (BaseClass && !CalledMethod) {
							CalledMethod = BaseClass->getMethods().lookup(Mangled);
							// If not found, go up the inheritance chain of this base class
							const auto &NextBases = BaseClass->getBaseClasses();
							BaseClass = !NextBases.empty() ? NextBases.front() : nullptr;
						}
						if (CalledMethod) break;
					}
                }

				// Create a call to class method
				if (CalledMethod) {
					Call->Sema = SemaBuilder::CreateCall(Call);

					// Set the Call Sema ErrorHandler
					ResolveErrorHandler(Stmt, Call->Sema);

					// Set the Call Sema Function
					Call->Sema->Function = CalledMethod;

					// Set First and Parent
					Call->Sema->setParent(Parent);

					if (Ref->Child)
						ResolveInstanceRef(Stmt, Ref->Child, Call->Sema);
				}
			}

			// class attribute
			else {

				// Set as Resolved
				Ref->Resolved = true;

				SemaClassAttribute *Attr = ClassType->getAttributes().lookup(Ref->getName());
				if (Attr) {

					SemaMemberVar *Sema;
					if (Attr->isStatic()) {
						Ref->Sema = Attr;
					} else {
						SemaMemberVar *Member = SemaBuilder::CreateMemberVar(Attr->getAST(), Parent);
						Member->Type = Attr->getType();
						Member->ClassAttribute = Attr;
						Ref->Sema = Member;
					}

					if (Ref->Child)
						ResolveInstanceRef(Stmt, Ref->Child, Sema);
				}
			}
		}

		// Enum
		else if (Parent->getType()->isEnum()) {
			ResoveEnumRef(Stmt, Ref, static_cast<SemaEnumType *>(Parent->getType()));
		}
	}
}

void Resolver::ResoveEnumRef(ASTStmt *Stmt, ASTRef *Ref, SemaEnumType *EnumType) {

	// Set as Resolved
	Ref->Resolved = true;

	// Enum Entry
	SemaVar *Entry = EnumType->getEntries().lookup(Ref->getName());
	if (Entry) {
		Ref->Sema = Entry;
	}
}

ASTRef *Resolver::getParentRef(fly::ASTRef *Ref) {
	// Set current with the Top Parent
	ASTRef *Parent = Ref;
	while (Parent->getParent() != nullptr) {
		Parent->getParent()->Child = Parent;
		Parent = Parent->getParent();
	}

	return Parent;
}


bool Resolver::ResolveRef(ASTStmt *Stmt, ASTCall *Call) {
	if (!Call->Resolved) {

		// Get Parent Ref
		ASTRef *Parent = getParentRef(Call);

		// Ref is a CurrentNameSpace ?
		SemaNameSpace * CurrentNameSpace = ResolveNameSpace(Parent);

		// Resolve from top-bottom
		ResolveFromTopRef(Stmt, Parent, CurrentNameSpace);
	}

	return Call->getSema() != nullptr;
}

bool Resolver::ResolveRef(ASTStmt *Stmt, ASTRef *VarRef) {
	if (!VarRef->Resolved) {

		// Get Parent Ref
		ASTRef *Parent = getParentRef(VarRef);

		// Ref is a CurrentNameSpace ?
		SemaNameSpace * CurrentNameSpace = ResolveNameSpace(Parent);

		// Resolve from top-bottom
		ResolveFromTopRef(Stmt, Parent, CurrentNameSpace);
	}

	return VarRef->getSema() != nullptr;
}

SemaNameSpace *Resolver::ResolveNameSpace(ASTRef *Ref) {
	// Ref is the current module namespace
	if (Ref->getName() == Module->getNameSpace()->getName()) {
		return Module->getNameSpace();
	}

	// Import CurrentNameSpace
	SemaNameSpace *CurrentNameSpace = nullptr;
	SemaNameSpace *ChildNameSpace = nullptr;
	std::string NameSpaceStr = "";
	ASTRef *Child = Ref;
	while (Child) {
		NameSpaceStr = NameSpaceStr.empty() ? Child->getName().data() : NameSpaceStr + "." + Child->getName().data();
		ChildNameSpace = Module->getImports().lookup(NameSpaceStr);
		if (ChildNameSpace) {
			CurrentNameSpace = ChildNameSpace;
		}
		Child = Child->getChild();
	}

	if (CurrentNameSpace)
		return CurrentNameSpace;

	return CurrentNameSpace;
}

SemaType * Resolver::ResolveType(llvm::StringRef Name, SemaNameSpace *CurrentNameSpace) {
	SemaType *Type = nullptr;

	if (CurrentNameSpace->getName() == Module->getNameSpace()->getName()) {
		// Search for private types in the current module
		Type = Module->getTypes().lookup(Name);
	}

	// Search for public types in current namespace
	if (!Type)
		Type = CurrentNameSpace->getTypes().lookup(Name);

	if (!Type && !isDefaultNameSpace)
		// Resolve in Default CurrentNameSpace
		Type = S.getSymTable().getDefaultNameSpace()->getTypes().lookup(Name);

	return Type;
}

void Resolver::ResolveErrorHandler(ASTStmt *Stmt, SemaCall *Sema) {
	// Search until parent is null or parent is a Handle Stmt
	// When Parent Stmt is nullptr assign Function ErrorHandler to Call ErrorHandler
	ASTStmt *Parent = Stmt;
	while (Sema->getErrorHandler() == nullptr) {
		Parent = Parent->getParent();
		if (Parent == nullptr) {
			Sema->ErrorHandler = Stmt->getFunction()->getSema()->getErrorHandler();
		} else if (Parent->getStmtKind() == ASTStmtKind::STMT_HANDLE) {
			ASTHandleStmt *HandleStmt = static_cast<ASTHandleStmt*>(Parent);
			if (HandleStmt->getErrorHandlerRef() != nullptr) {
				Sema->ErrorHandler = reinterpret_cast<SemaErrorHandler *>(HandleStmt->getErrorHandlerRef()->Sema);
			}
		}
	}
}

/**
 * Resolve a Call Reference
 * @param Stmt
 * @param Call
 * @param NameSpaces
 * @param ...
 * @return
 */
SemaCall *Resolver::ResolveCall(ASTStmt *Stmt, ASTCall *Call, SemaNameSpace *CurrentNameSpace) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveCall", Logger().Attr("Call", Call).End());
    assert(Stmt && "Stmt cannot be null");
    assert(Call && "Call cannot be null");

    // Resolve Expression in Arguments
    llvm::SmallVector<SemaType *, 8> TypeArgs = ResolveCallArgTypes(Stmt, Call);

    // if Arguments are not resolved is not possible go ahead with call reference resolution
    // cannot resolve with the function parameters types
    std::string Mangled = SemaFunctionBase::MangleFunction(Call->Name, TypeArgs);

	// Set as Resolved: TODO check if Resolve == false at start
	Call->Resolved = true;
	SemaCall *Sema = nullptr;

	// Create a new instance using a Constructor
    if (Call->getCallKind() == ASTCallKind::CALL_NEW ||
        Call->getCallKind() == ASTCallKind::CALL_NEW_UNIQUE ||
        Call->getCallKind() == ASTCallKind::CALL_NEW_SHARED ||
        Call->getCallKind() == ASTCallKind::CALL_NEW_WEAK) {

        // Take the Type from the CurrentNameSpace
        SemaType *Type = ResolveType(Call->getName(), CurrentNameSpace);

    	// No type found, no constructor
    	if (Type == nullptr) {
    		Diag(Call->getLocation(), diag::err_unref_type);
    		return nullptr;
    	}

    	// Call Constructor
        if (!Type->isClass()) {
        	Diag(Call->getLocation(), diag::err_unref_type);
        	return nullptr;
        }

		SemaClassType *Class = static_cast<SemaClassType *>(Type);
		SemaClassMethod *Constructor = Class->getConstructors().lookup(Mangled);

    	if (Constructor == nullptr) {
    		// Error: func not found
    		Diag(Call->getLocation(), diag::err_syntax_error);
    		return nullptr;
    	}

        Sema = SemaBuilder::CreateCall(Call);
        Sema->Function = Constructor;
    	Sema->Type = Type;
    } else {

    	// Call a Function or a Class Method
    	SemaFunctionBase *Func = nullptr;
    	if (Stmt->getFunction()->getSema()->getKind() == SemaFunctionKind::CLASS_METHOD) {

    		// Check if the Call is a Base Class Constructor Method
    		SemaClassType *CurrentClass = static_cast<SemaClassMethod *>(Stmt->getFunction()->getSema())->getClass();
    		SemaType * T = CurrentNameSpace ?
			            CurrentNameSpace->getTypes().lookup(Call->Name) :
			            CurrentNameSpace->getTypes().lookup(Call->Name);
    		if (T->isClass()) {
				SemaClassType * C = static_cast<SemaClassType *>(T);

    			// Call to Base Class Constructor Method
    			if (C->isBaseOrEquals(CurrentClass)) {
    				// Resolve Call with Class Constructor if is not private
    				SemaClassMethod *Constructor = C->getConstructors().lookup(Mangled);

    				// check if Constructor is private
    				if (Constructor->getVisibility() == SemaVisibilityKind::PRIVATE) {
    					// Error: method is private, cannot be called from outside the class
    					Diag(Call->getLocation(), diag::err_syntax_error);
    					return nullptr;
    				}
    				// Take the Constructor
    				Func = Constructor;
    			} else {
    				// Static Call to Class Method
    				SemaClassMethod *Method = C->getMethods().lookup(Mangled);

    				// Check if Method is static
    				if (!Method->isStatic()) {
    					// Error: method is not static, cannot be called statically
    					Diag(Call->getLocation(), diag::err_syntax_error);
    				}

    				// Check if Method is private
    				if (Method->getVisibility() == SemaVisibilityKind::PRIVATE) {
    					// Error: method is private, cannot be called from outside the class
    					Diag(Call->getLocation(), diag::err_syntax_error);
    				}

    				// Take the Method
    				Func = Method;
    			}
    		}
    	}

    	// Take the Function
    	if (Func == nullptr) {
    		Func = CurrentNameSpace ?
				CurrentNameSpace->getFunctions().lookup(Mangled) :
				CurrentNameSpace->getFunctions().lookup(Mangled);
    	}

    	if (Func == nullptr) {
    		// Error: func not found
    		Diag(Call->getLocation(), diag::err_syntax_error);
    		return nullptr;
    	}

    	Sema = SemaBuilder::CreateCall(Call);
        Sema->Function = Func;
    	Sema->Type = Func->getReturnType();
    }

	// Set the Call Sema ErrorHandler
	ResolveErrorHandler(Stmt, Sema);

	Sema->AST = Call;
	Call->Sema = Sema;
	return Sema;
}

llvm::SmallVector<SemaType *, 8> Resolver::ResolveCallArgTypes(ASTStmt *Stmt, ASTCall *Call) {
	// Resolve Expression in Arguments
	llvm::SmallVector<SemaType *, 8> CallTypes;
	for (auto Arg : Call->getArgs()) {
		ResolveExpr(Stmt, Arg->Expr);
		CallTypes.push_back(Arg->getExpr()->getType());
	}
	return CallTypes;
}

SemaVar *Resolver::ResolveVar(ASTStmt *Stmt, ASTRef *VarRef) {
	SemaVar *Sema = nullptr;

	// Skip already resolved VarRef
	if (VarRef->getSema() != nullptr) {
		Sema = static_cast<SemaVar *>(VarRef->getSema());
	}

	// Search into Local Vars
	if (!Sema && Stmt->getStmtKind() == ASTStmtKind::STMT_BLOCK) {
		ASTBlockStmt *Block = static_cast<ASTBlockStmt*>(Stmt);
		const auto &It = Block->getLocalVars().find(VarRef->getName());
		if (It != Block->getLocalVars().end()) { // Search into this Block
			Sema = It->getValue()->getSema();
		}
	}

	// Search in parent Stmt
	if (!Sema && Stmt->getParent()) {
		// search recursively into Parent Stmt to find the right Var definition
		SemaVar * ParentVar = ResolveVar(Stmt->getParent(), VarRef);
		if (ParentVar) {
			Sema = ParentVar;
		}
	}

	// Search into Function Parameter list
	if (!Sema) {
		llvm::SmallVector<ASTVar *, 8> Params = Stmt->getFunction()->getParams();
		for (auto &Param : Params) {
			if (Param->getName() == VarRef->getName()) {
				Sema = Param->getSema();
				break;
			}
		}
	}

	// Search into Class Attributes
	bool isSemaAttribute = false;
	if (!Sema && Stmt->getFunction()->getSema()->getKind() == SemaFunctionKind::CLASS_METHOD) {
		// Get the Class Type
		SemaClassMethod *Method = static_cast<SemaClassMethod *>(Stmt->getFunction()->getSema());
		if (!Method->isStatic()) {
			if (VarRef->getName() == "this") {
				Sema = Method->getClass()->getThis();
			} else {
				Sema = Method->getClass()->getAttributes().lookup(VarRef->getName());
			}
			if (Sema) {
				isSemaAttribute = true; // VarRef is a Class Attribute
			}
		}
	}

	// TODO: remove globalvars
	// Search into Global Vars
	// if (!Var) {
	// 	for (auto NS : NameSpaces) {
	// 		Var = NS->getGlobalVars().lookup(Ref->getName());
	// 		if (Var)
	// 			break;
	// 	}
	// }

	if (Sema == nullptr) {
		// Error: var not found
		Diag(VarRef->getLocation(), diag::err_syntax_error);
	}

	// Add Var to LocalVars of the SemaFunctionBase
	if (!isSemaAttribute)
		Stmt->getFunction()->getSema()->getLocalVars().push_back(Sema); // Function Local var to be allocated

	VarRef->Sema = Sema;
	VarRef->Resolved = true;
	return Sema;
}
