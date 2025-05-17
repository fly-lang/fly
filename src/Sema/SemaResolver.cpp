//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaResolver.cpp - The Sema Resolver
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaResolver.h"
#include "Sema/SemaResolverClass.h"
#include "Sema/Sema.h"
#include "Sema/ASTBuilder.h"
#include "Sema/SemaBuilder.h"
#include "Sema/SemaValidator.h"
#include "Basic/Logger.h"
#include "Sema/SymTable.h"
#include "Sema/SemaNameSpace.h"
#include "Sema/SemaModule.h"
#include "Sema/SemaClassType.h"
#include "Sema/SemaClassMethod.h"
#include "Sema/SemaClassAttribute.h"
#include "Sema/SemaEnumType.h"
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
#include "AST/ASTVarRef.h"
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

using namespace fly;

SemaResolver::SemaResolver(Sema &S, ASTModule *Module) :
    S(S), NameSpace(S.getSemaBuilder().CreateOrGetNameSpace(Module->getNameSpace())),
	Module(S.getSemaBuilder().CreateModule(NameSpace, Module)),
	isDefaultNameSpace(NameSpace->getName() == Sema::DEFAULT_NAMESPACE) {

}

/**
 * Resolve Modules by creating the right structure for resolving all symbols
 */
bool SemaResolver::Resolve(Sema &S) {
    llvm::SmallVector<SemaResolver *, 8> Resolvers;

    // First: Resolve Modules for populate NameSpaces
    for (auto &Module : S.getModules()) {

    	// Validate Module name duplication
    	if (S.getValidator().CheckDuplicateModules(Module)) {

    		// Resolve Declarations
    		SemaResolver *Resolver = new SemaResolver(S, Module);
    		Resolver->AddSymbols();

    		// add to Resolvers list
    		Resolvers.push_back(Resolver);
    	}
    }

    // Second: Resolve Definitions
    for (auto &Resolver : Resolvers) {
    	Resolver->ResolveImports();
    	// TODO: remove GlobalVar
		// Resolver->ResolveGlobalVars();
    	Resolver->ResolveTypes();
    	Resolver->ResolveFunctions();
    }

	// Third: Resolve Bodies
	for (auto &Resolver : Resolvers) {
		Resolver->ResolveBodies();
	}

    return !S.Diags.hasErrorOccurred();
}

void SemaResolver::AddSymbols() {
	Module->Imports.insert(std::make_pair(NameSpace->getName(), NameSpace));

	SemaComment *Comment = nullptr;
	for (ASTBase *AST : Module->getAST()->getDefinitions()) {

		// Set Comment if the previous AST is a Comment
		switch (AST->getKind()) {
			case ASTKind::AST_IMPORT: {
				S.getSemaBuilder().CreateImport(Module, static_cast<ASTImport *>(AST));
				Comment = nullptr;
			} break;
			// TODO: remove GlobalVar
			// case ASTKind::AST_VAR: {
			//  SemaGlobalVar *GlobalVar = S.getSemaBuilder().CreateGlobalVar(Module, static_cast<ASTVar *>(AST));
			//  GlobalVar->Comment = Comment;
			// 	Comment = nullptr;
			// } break;
			case ASTKind::AST_FUNCTION: {
				SemaFunction *Function = S.getSemaBuilder().CreateFunction(Module, static_cast<ASTFunction *>(AST));
				Function->Comment = Comment;
				Comment = nullptr;
			} break;
			case ASTKind::AST_CLASS: {
				SemaClassType * Class = S.getSemaBuilder().CreateClass(Module, static_cast<ASTClass *>(AST));
				Class->Comment = Comment;
				Comment = nullptr;
			} break;
			case ASTKind::AST_ENUM: {
				SemaEnumType *Enum = S.getSemaBuilder().CreateEnum(Module, static_cast<ASTEnum *>(AST));
				Enum->Comment = Comment;
				Comment = nullptr;
			} break;
			case ASTKind::AST_COMMENT:
				Comment = S.getSemaBuilder().CreateComment(static_cast<ASTComment *>(AST));
				break;
			default:
                // Error: invalid declaration in module
                S.Diag(AST->getLocation(), diag::err_syntax_error);
                break;
		}

	}
}

/**
 * ResolveModule Import Definitions
 */
void SemaResolver::ResolveImports() {
	for (auto &Entry : Module->Imports) {

		SemaNameSpace *Import = S.getSymTable().getNameSpaces().lookup(Entry.getKey());
		if (!Import) {
			S.Diag(diag::err_namespace_notfound) << Entry.getKey();
			return;
		}

		Entry.setValue(Import);
	}
}

void SemaResolver::ResolveComment(SemaComment *Comment, ASTBase* AST) {
	if (AST->getKind() == ASTKind::AST_FUNCTION) {
		ASTFunction * Function = (ASTFunction *) AST;
		S.getValidator().CheckCommentParams(Comment, Function->getParams());
		S.getValidator().CheckCommentReturn(Comment, Function->getReturnTypeRef());
		S.getValidator().CheckCommentFail(Comment);
	}
}

/**
 * Resolve Module GlobalVar Definitions
 */
// TODO: remove GlobalVar
// void SemaResolver::ResolveGlobalVars() {
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
// 			S.Diag(AST->Expr->getLocation(), diag::err_invalid_gvar_value);
// 		}
//
// 		// Resolve Type
// 		if (ResolveTypeRef(AST->TypeRef)) {
// 			Sema->Type = AST->TypeRef->getSema();
// 		}
// 	}
// }

/**
 * Resolve Module Function Definitions
 */
void SemaResolver::ResolveFunctions() {
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
			//S.getValidator().CheckDuplicateParams(Function->Params, Param);

			// resolve parameter type
			if (ResolveTypeRef(Param->TypeRef)) {
				SemaParam *P = S.getSemaBuilder().CreateParam(Param);
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
void SemaResolver::ResolveTypes() {
	for (auto &TypeEntry : Module->getTypes()) {
		SemaType *Sema = TypeEntry.getValue();

		if (Sema->isClass()) {
			SemaClassType * ClassType = static_cast<SemaClassType *>(Sema);

			// Resolve Comment
			if (ClassType->Comment) {
				ResolveComment(ClassType->Comment, ClassType->getAST());
			}

			// Resolve Class Type
			SemaResolverClass::Resolve(this, ClassType);

		} else if (Sema->isEnum()) {
			SemaEnumType * EnumType = static_cast<SemaEnumType *>(Sema);

			// Resolve Comment
			if (EnumType->Comment) {
				ResolveComment(EnumType->Comment, EnumType->getAST());
			}

			// Resolve Enum Type
			ResolveEnumType(EnumType);
		}

		NameSpace->Types.insert(std::make_pair(Sema->getName(), Sema));
	}
}

void SemaResolver::ResolveBodies() {
	for (ASTBlockStmt *Body : Bodies) {

		// Resolve Function Body
		ResolveStmtBlock(Body);
	}
}

void SemaResolver::ResolveEnumType(SemaEnumType *Sema) {
	if (Sema->Comment) {
		ResolveComment(Sema->Comment, Sema->getAST());
	}

	SemaComment *Comment = nullptr;
	for (auto &AST: Sema->getAST()->getDefinitions()) {
		switch (AST->getKind()) {
			case ASTKind::AST_VAR: {
				S.getSemaBuilder().CreateEnumEntry(Sema, static_cast<ASTVar *>(AST), Comment);
				Comment = nullptr;
			}	break;
			case ASTKind::AST_COMMENT:
				Comment = S.getSemaBuilder().CreateComment(static_cast<ASTComment *>(AST));
			break;
			default:
				// Error: invalid declaration in class
					S.Diag(AST->getLocation(), diag::err_syntax_error);
			break;
		}
	}
}

bool SemaResolver::ResolveStmt(ASTStmt *Stmt) {
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
            		//S.getValidator().CheckConvertibleTypes(, ReturnType->getSym());
            		ReturnStmt->Expr->Type = ReturnType->getSema();
            	}
            } else {
            	if (!ReturnStmt->Parent->getFunction()->getReturnTypeRef()->getSema()->isVoid()) {
            		S.Diag(ReturnStmt->getLocation(), diag::err_invalid_return_type);
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

bool SemaResolver::ResolveStmtBlock(ASTBlockStmt *Block) {
    // Resolve LocalVar Type
    for (auto &VarEntry : Block->LocalVars) {
    	ASTVar * LocalVar = VarEntry.getValue();

    	// Resolve LocalVar Type
        ResolveTypeRef(LocalVar->TypeRef);

    	// Create LocalVar Sema
    	SemaLocalVar * Sema = S.getSemaBuilder().CreateLocalVar(LocalVar);

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
//            S.Diag(LocalVar.getValue()->getLocation(), diag::err_sema_uninit_var) << LocalVar.getValue()->getName();
//    }

    return true;
}

bool SemaResolver::ResolveStmtIf(ASTIfStmt *IfStmt) {
    bool Success = ResolveExpr(IfStmt->getParent(), IfStmt->Rule);
	IfStmt->Rule->Type = S.getSymTable().getBoolType();

    Success &= ResolveStmt(IfStmt->Stmt);
    for (ASTRuleStmt *Elsif : IfStmt->Elsif) {
        Success &= ResolveExpr(IfStmt->getParent(), Elsif->Rule);
    	Elsif->Rule->Type = S.getSymTable().getBoolType();
        Success &= ResolveStmt(Elsif->Stmt);
    }
    if (Success && IfStmt->Else) {
        Success = ResolveStmt(IfStmt->Else);
    }
    return Success;
}

bool SemaResolver::ResolveStmtSwitch(ASTSwitchStmt *SwitchStmt) {
    assert(SwitchStmt && "Switch Block cannot be null");

    bool Success = ResolveRef(SwitchStmt->getParent(), SwitchStmt->VarRef) &&
    	SwitchStmt->getVarRef()->getSema()->getType()->isInteger();
    for (ASTRuleStmt *Case : SwitchStmt->Cases) {
    	Success &= ResolveExpr(SwitchStmt, Case->getRule());
        Success &= Case->getRule()->getType()->isInteger() && ResolveStmt(Case);
    }
    return Success && ResolveStmt(SwitchStmt->Default);
}

bool SemaResolver::ResolveStmtLoop(ASTLoopStmt *LoopStmt) {
    // Check Loop is not null or empty
    if (LoopStmt->Stmt == nullptr) {
    	S.Diag(diag::err_sema_generic);
    	return false;
    }

	if (LoopStmt->getRule() == nullptr) { // Error: empty condition expr
		S.Diag(diag::err_parse_empty_while_expr);
		return false;
	}

	bool Success = true;
    // Check Init
    if (LoopStmt->Init) {
        LoopStmt->Stmt->Parent = LoopStmt->Init;
        Success = ResolveStmt(LoopStmt->Init);
        Success &= ResolveExpr(LoopStmt->Init, LoopStmt->Rule);
    	LoopStmt->Rule->Type = S.getSymTable().getBoolType();
    } else {
        Success = ResolveExpr(LoopStmt->Parent, LoopStmt->Rule);
    	LoopStmt->Rule->Type = S.getSymTable().getBoolType();
    }
    Success = S.getValidator().CheckConvertibleTypes(LoopStmt->getRule()->getType(), S.getSymTable().getBoolType());
    Success &= ResolveStmt(LoopStmt->Stmt);
    Success &= LoopStmt->Post ? ResolveStmt(LoopStmt->Post) : true;
    return Success;
}

bool SemaResolver::ResolveStmtLoopIn(ASTLoopInStmt *LoopInStmt) {
    return ResolveRef(LoopInStmt->Parent, LoopInStmt->VarRef) && ResolveStmtBlock(LoopInStmt->Block);
}

bool SemaResolver::ResolveStmtVar(ASTVarStmt *VarStmt) {
    if (ResolveRef(VarStmt->Parent, VarStmt->VarRef)) {
	    SemaVar *Var = VarStmt->getVarRef()->getSema();
    	if (Var && VarStmt->getExpr() != nullptr) {
    		if (ResolveExpr(VarStmt->Parent, VarStmt->Expr)) {
    			VarStmt->Expr->Type = Var->getType();
    			return true;
    		}
    	}
    }
	return false;
}

bool SemaResolver::ResolveStmtFail(ASTFailStmt *FailStmt) {
	// Resolve Fail Expr
	if (FailStmt->Expr)
		return ResolveExpr(FailStmt->Parent, FailStmt->Expr);

    return true;
}

bool SemaResolver::ResolveStmtHandle(ASTHandleStmt *HandleStmt) {
    if (HandleStmt->ErrorHandlerRef)
        ResolveRef(HandleStmt->getParent(), HandleStmt->ErrorHandlerRef);
    return ResolveStmt(HandleStmt->Handle);
}

bool SemaResolver::ResolveValue(ASTValue *AST) {
	assert(AST && "Value cannot be null");
	switch (AST->getTypeKind()) {

		// Bool Value
		case ASTValueKind::VAL_BOOL:
			return S.SBuilder->CreateBoolValue(static_cast<ASTBoolValue *>(AST));

		// Number Value
		case ASTValueKind::VAL_NUMBER:
			return S.SBuilder->CreateNumberValue(static_cast<ASTNumberValue *>(AST));

		// String Value
		case ASTValueKind::VAL_STRING:
			return S.SBuilder->CreateStringValue(static_cast<ASTStringValue *>(AST));

		// Array Value
		case ASTValueKind::VAL_ARRAY: {
			ASTArrayValue * ArrayAST = static_cast<ASTArrayValue *>(AST);
			SemaArrayValue *Array = S.SBuilder->CreateArrayValue(ArrayAST);
			for (auto Value : ArrayAST->getValues()) {
				if (ResolveValue(Value))
					Array->Values.push_back(Value->getSema());
			}
			return Array;
		}

		// Struct Value
		case ASTValueKind::VAL_STRUCT: {
			ASTStructValue * StructAST = static_cast<ASTStructValue *>(AST);
			SemaStructValue *Struct = S.SBuilder->CreateStructValue(StructAST);
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
bool SemaResolver::ResolveExpr(ASTStmt *Stmt, ASTExpr *Expr) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveExpr", Logger()
            .Attr("Expr", Expr)
            .Attr("Type", Expr).End());

    bool Success = false;
    switch (Expr->getExprKind()) {
        case ASTExprKind::EXPR_VALUE: {
            ASTValue *Value = static_cast<ASTValueExpr*>(Expr)->getValue();

        	// Select the best Value between: bool, number, string, array, struct
        	if (ResolveValue(Value)) {
        		Expr->Type = Value->getSema()->getType();
        		return true;
        	}
        }
        case ASTExprKind::EXPR_VAR_REF: {
            ASTVarRef *VarRef = static_cast<ASTVarRefExpr*>(Expr)->getVarRef();

        	// Start to resolve the VarRef from root
            if (ResolveRef(Stmt, VarRef)) {
                Expr->Type = VarRef->getSema()->getType();
                return true;
            }
        }
        case ASTExprKind::EXPR_CALL: {
            ASTCall *Call = static_cast<ASTCallExpr*>(Expr)->getCall();

        	// Start to resolve the CallRef from root
            if (ResolveRef(Stmt, Call)) {
                switch (Call->getCallKind()) {

                    case ASTCallKind::CALL_FUNCTION:
                        Expr->Type = Call->getSema()->getFunction()->getReturnType();
                        break;
                    case ASTCallKind::CALL_NEW: {
                    	SemaClassMethod *Method = static_cast<SemaClassMethod *>(Call->getSema()->getFunction());
                        Expr->Type = Method->getClass();
                    }
                    break;
                }
                Success = true;
                break;
            }
            return false;
        }
        case ASTExprKind::EXPR_OP: {
            switch (static_cast<ASTOpExpr*>(Expr)->getOpExprKind()) {
                case ASTOpExprKind::OP_UNARY: {
                    ASTUnaryOpExpr *Unary = static_cast<ASTUnaryOpExpr*>(Expr);
                    Success = ResolveExpr(Stmt, const_cast<ASTExpr*>(Unary->Expr));
                    Expr->Type = Unary->getExpr()->getType();
                    break;
                }
                case ASTOpExprKind::OP_BINARY: {
                    ASTBinaryOpExpr *Binary = static_cast<ASTBinaryOpExpr*>(Expr);

                    Success = ResolveExpr(Stmt, Binary->LeftExpr) && ResolveExpr(Stmt, Binary->RightExpr);
                    if (Success) {

                    	// Check if Left and Right Expr are resolved
                    	SemaType * LeftType = Binary->getLeftExpr()->getType();
                    	SemaType * RightType = Binary->getRightExpr()->getType();

                        if (Binary->getTypeKind() == ASTBinaryOpTypeExprKind::OP_BINARY_ARITH ||
                                Binary->getTypeKind() == ASTBinaryOpTypeExprKind::OP_BINARY_COMPARISON) {

                        	// Check Compatible Types Bool/Bool, Float/Float, Integer/Integer
                            Success = S.getValidator().CheckArithTypes(Binary->getLeftExpr()->getType(),
                                                                  Binary->getRightExpr()->getType());

                            if (Success) {
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
                                               LeftType : S.getSymTable().getBoolType();
                            } else {
                            	S.Diag(Binary->getLocation(), diag::err_sema_types_operation)
											<< LeftType->getName()
											<< RightType->getName();
                            }
                        } else if (Binary->getTypeKind() == ASTBinaryOpTypeExprKind::OP_BINARY_LOGIC) {
                            Success = S.getValidator().CheckLogicalTypes(LeftType, RightType);
                        	if (Success) {
                        		Binary->Type = S.getSymTable().getBoolType();
                        	} else {
                        		S.Diag(Binary->getLocation(), diag::err_sema_types_logical)
									<< LeftType->getName()
									<< RightType->getName();
                        	}
                        }
                    }
                    break;
                }
                case ASTOpExprKind::OP_TERNARY: {
                    ASTTernaryOpExpr *Ternary = static_cast<ASTTernaryOpExpr*>(Expr);
                    Success = ResolveExpr(Stmt, Ternary->ConditionExpr) &&
                              S.getValidator().CheckConvertibleTypes(Ternary->getConditionExpr()->getType(), S.getSymTable().getBoolType()) &&
                              ResolveExpr(Stmt, Ternary->TrueExpr) &&
                              ResolveExpr(Stmt, Ternary->FalseExpr);
                    Ternary->Type = Ternary->getTrueExpr()->getType(); // The group type is equals to the second type
                    break;
                }
            }
            break;
        }
        default:
            assert(0 && "Invalid ASTExprKind");
    }

    return Success;
}

bool SemaResolver::ResolveTypeRef(ASTTypeRef *&TypeRef) {
	if (!TypeRef->Resolved) {

		// Set current with the Top Parent
		ASTRef *Current = TypeRef;
		while (Current->getParent() != nullptr) {
			Current->getParent()->Child = Current;
			Current = Current->getParent();
		}

		// Ref is a NameSpace ?
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

		// Take Identity from NameSpace
		TypeRef->Resolved = TypeRef->Sema != nullptr; // Evict Cycle Loop: can be resolved only now
	}

	if (!TypeRef->Sema) {
		S.Diag(TypeRef->getLocation(), diag::err_unref_type);
		return false;
	}

	return true;
}

ASTRef *SemaResolver::getParentRef(fly::ASTRef *Ref) {
	// Set current with the Top Parent
	ASTRef *Parent = Ref;
	while (Parent->getParent() != nullptr) {
		Parent->getParent()->Child = Parent;
		Parent = Parent->getParent();
	}

	return Parent;
}

bool SemaResolver::ResolveRef(ASTStmt *Stmt, ASTVarRef *VarRef) {
	if (!VarRef->Resolved) {

		// Get Parent Ref
		ASTRef *Parent = getParentRef(VarRef);

		// Ref is a NameSpace ?
		SemaNameSpace * CurrentNameSpace = ResolveNameSpace(Parent);

		// Resolve from top-bottom
		ResolveRef(Stmt, Parent, CurrentNameSpace);
	}

	return VarRef->getSema() != nullptr;
}

bool SemaResolver::ResolveRef(ASTStmt *Stmt, ASTCall *Call) {
	if (!Call->Resolved) {

		// Get Parent Ref
		ASTRef *Parent = getParentRef(Call);

		// Ref is a NameSpace ?
		SemaNameSpace * CurrentNameSpace = ResolveNameSpace(Parent);

		// Resolve from top-bottom
		ResolveRef(Stmt, Parent, CurrentNameSpace);
	}

	return Call->getSema() != nullptr;
}

/**
 * Resolve a Reference, continue to resolve until the Ref is completely resolved
 * @param Stmt
 * @param Ref
 * @param NameSpaces
 * @param ...
 * @return
 */
void SemaResolver:: ResolveRef(ASTStmt *Stmt, ASTRef *Ref, SemaNameSpace *CurrentNameSpace) {
	if (!Ref->Resolved) {

		// Ref is a Function
		if (Ref->isCall()) {
			ASTCall *CallRef = static_cast<ASTCall *>(Ref);
			SemaCall *Sema = ResolveCall(Stmt, CallRef, CurrentNameSpace);

			// TODO
			if (CallRef->Child)
				ResolveRef(Stmt, CallRef->Child, Sema->getFunction()->getReturnType(), Sema);
		}

		//Ref is a Var
		else if (Ref->isVarRef()) {
			ASTVarRef *VarRef = static_cast<ASTVarRef *>(Ref);
			SemaVar *Sema = ResolveVar(Stmt, VarRef);

			if (Ref->Child)
				ResolveRef(Stmt, Ref->Child, Sema->getType(), Sema);
		}

		// Ref is a Class or an Enum Type?
		else {
			SemaType *Type = ResolveType(Ref->getName(), CurrentNameSpace);
			if (Type) {
				Ref->Resolved = true;
				ResolveRef(Stmt, Ref->Child, Type, nullptr);
				return;
			}

			// Ref is a Var
			ASTVarRef * VarRef = S.getASTBuilder().CreateVarRef(Ref); // FIXME check if is varref?
			SemaVar *Sema = ResolveVar(Stmt, VarRef);

			// TODO
			if (Ref->Child)
				return ResolveRef(Stmt, Ref->Child, Sema->getType(), Sema);
		}
	}
}


/**
 * Resolve static Ref to a Class or Enum
 * @param Type
 * @param Ref
 * @return
 */
void SemaResolver:: ResolveRef(ASTStmt *Stmt, ASTRef *Ref, SemaType *Type, SemaResult *Parent) {
	if (!Ref->Resolved) {

		// Class
		if (Type->isClass()) {
			SemaClassType *ClassType = static_cast<SemaClassType *>(Type);

			// static call
			if (Ref->isCall()) {
				ASTCall *Call = static_cast<ASTCall *>(Ref);
				SmallVector<SemaType *, 8> CallTypes = ResolveCallArgTypes(Stmt, Call);
				std::string Mangled = SemaFunctionBase::MangleFunction(Call->getName(), CallTypes);
				SemaFunctionBase* Method = ClassType->getMethods().lookup(Mangled);
				if (Method) {
					Call->Sema = S.getSemaBuilder().CreateCall(Call);
					Call->Sema->Function = Method;
					Call->Resolved = true;
					Call->Sema->Parent = Parent;
					if (Ref->Child)
						ResolveRef(Stmt, Ref->Child, Method->getReturnType(), Call->Sema);
				}
			} else { // static var
				SemaVar *Sema = ClassType->getAttributes().lookup(Ref->getName());
				if (Sema && static_cast<SemaClassAttribute *>(Sema)->isStatic()) {
					Ref = S.getASTBuilder().CreateVarRef(Ref);
					static_cast<ASTVarRef *>(Ref)->Sema = Sema;
					Sema->Parent = Parent;
					Ref->Resolved = true;
					if (Ref->Child)
						ResolveRef(Stmt, Ref->Child, Sema->getType(), Sema);
				}
			}
		}

		// Enum
		else if (Type->isEnum()) {
			SemaEnumType *EnumType = static_cast<SemaEnumType *>(Type);

			// Enum Entry
			SemaVar *Entry = EnumType->getEntries().lookup(Ref->getName());
			if (Entry) {
				Ref = S.getASTBuilder().CreateVarRef(Ref);
				static_cast<ASTVarRef *>(Ref)->Sema = Entry;
				Ref->Resolved = true;
			}
		}
	}
}

SemaNameSpace *SemaResolver::ResolveNameSpace(ASTRef *Ref) {
	// Ref is the current module namespace
	if (Ref->getName() == Module->getNameSpace()->getName()) {
		return Module->getNameSpace();
	}

	// Import NameSpace
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
		Child = Ref->getChild();
	}

	if (CurrentNameSpace)
		return CurrentNameSpace;

	return NameSpace;
}

SemaType * SemaResolver::ResolveType(llvm::StringRef Name, SemaNameSpace *CurrentNameSpace) {
	SemaType *Type = nullptr;

	if (CurrentNameSpace->getName() == Module->getNameSpace()->getName()) {
		// Search for private types in the current module
		Type = Module->getTypes().lookup(Name);
	}

	// Search for public types in current namespace
	if (!Type)
		Type = CurrentNameSpace->getTypes().lookup(Name);

	if (!Type && !isDefaultNameSpace)
		// Resolve in Default NameSpace
		Type = S.getSymTable().getDefaultNameSpace()->getTypes().lookup(Name);

	return Type;
}

/**
 * Resolve a Call Reference
 * @param Stmt
 * @param Call
 * @param NameSpaces
 * @param ...
 * @return
 */
SemaCall *SemaResolver::ResolveCall(ASTStmt *Stmt, ASTCall *Call, SemaNameSpace *CurrentNameSpace) {
    FLY_DEBUG_MESSAGE("Sema", "ResolveCall", Logger().Attr("Call", Call).End());
    assert(Stmt && "Stmt cannot be null");
    assert(Call && "Call cannot be null");

    // Resolve Expression in Arguments
    llvm::SmallVector<SemaType *, 8> TypeArgs = ResolveCallArgTypes(Stmt, Call);

    // if Arguments are not resolved is not possible go ahead with call reference resolution
    // cannot resolve with the function parameters types
    std::string Mangled = SemaFunctionBase::MangleFunction(Call->Name, TypeArgs);

	SemaCall *Sema = nullptr;

    // Constructor
    if (Call->getCallKind() == ASTCallKind::CALL_NEW ||
        Call->getCallKind() == ASTCallKind::CALL_NEW_UNIQUE ||
        Call->getCallKind() == ASTCallKind::CALL_NEW_SHARED ||
        Call->getCallKind() == ASTCallKind::CALL_NEW_WEAK) {

        // Take the Type from the NameSpace
        SemaType *Type = ResolveType(Call->getName(), CurrentNameSpace);

    	// No type found, no constructor
    	if (Type == nullptr) {
    		S.Diag(Call->getLocation(), diag::err_unref_type);
    		return nullptr;
    	}

    	// Call Constructor
        if (!Type->isClass()) {
        	S.Diag(Call->getLocation(), diag::err_unref_type);
        	return nullptr;
        }

		SemaClassType *Class = static_cast<SemaClassType *>(Type);
		SemaClassMethod *Constructor = Class->getConstructors().lookup(Mangled);
        Sema = S.getSemaBuilder().CreateCall(Call);
        Sema->Function = Constructor;
    } else {

        // Take the Function
        SemaFunction *Func = CurrentNameSpace ?
        	CurrentNameSpace->getFunctions().lookup(Mangled) :
        	NameSpace->getFunctions().lookup(Mangled);
    	Sema = S.getSemaBuilder().CreateCall(Call);
        Sema->Function = Func;
    }

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

	Sema->AST = Call;
	Call->Sema = Sema;
	Call->Resolved = true;
	return Sema;
}


SemaVar *SemaResolver::ResolveVar(ASTStmt *Stmt, ASTVarRef *VarRef) {
	SemaVar *Sema = nullptr;

	if (VarRef->getVar() != nullptr  && VarRef->getVar()->getSema() != nullptr) {
		Sema = VarRef->getVar()->getSema();
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
		S.Diag(VarRef->getLocation(), diag::err_syntax_error);
	}

	// Add Var to LocalVars of the SemaFunctionBase
	Stmt->getFunction()->getSema()->getLocalVars().push_back(Sema); // Function Local var to be allocated

	VarRef->Sema = Sema;
	VarRef->Resolved = true;
	return Sema;
}

llvm::SmallVector<SemaType *, 8> SemaResolver::ResolveCallArgTypes(ASTStmt *Stmt, ASTCall *Call) {
	// Resolve Expression in Arguments
	llvm::SmallVector<SemaType *, 8> CallTypes;
	for (auto Arg : Call->getArgs()) {
		ResolveExpr(Stmt, Arg->Expr);
		CallTypes.push_back(Arg->getExpr()->getType());
	}
	return CallTypes;
}

