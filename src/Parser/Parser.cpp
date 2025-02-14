//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/Parser.cpp - Main Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/Parser.h"
#include "Parser/ParserFunction.h"
#include "Parser/ParserExpr.h"
#include "Parser/ParserClass.h"
#include "Parser/ParserEnum.h"
#include "AST/ASTModule.h"
#include "AST/ASTClass.h"
#include "AST/ASTEnum.h"
#include "AST/ASTFunction.h"
#include "AST/ASTComment.h"
#include "AST/ASTCall.h"
#include "AST/ASTExpr.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTBlockStmt.h"
#include "AST/ASTContinueStmt.h"
#include "AST/ASTBreakStmt.h"
#include "AST/ASTValue.h"
#include "AST/ASTImport.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTTypeRef.h"
#include "AST/ASTVar.h"
#include "Sema/ASTBuilder.h"
#include "Sema/SemaBuilderScopes.h"
#include "Sema/SemaBuilderStmt.h"
#include "Sema/SemaBuilderIfStmt.h"
#include "Sema/SemaBuilderSwitchStmt.h"
#include "Sema/SemaBuilderLoopStmt.h"
#include "Frontend/InputFile.h"
#include "Basic/Debug.h"
#include "llvm/Support/Regex.h"

#include <AST/ASTScopes.h>

using namespace fly;

/**
 * Parser Constructor
 * @param Input
 * @param SourceMgr
 * @param Diags
 */
Parser::Parser(const InputFile &Input, SourceManager &SourceMgr, DiagnosticsEngine &Diags, ASTBuilder &Builder) :
    Input(Input), Diags(Diags), SourceMgr(SourceMgr), Builder(Builder),
    Lex(Input.getFileID(), Input.getBuffer(), SourceMgr) {
}

/**
 * Start Parsing Input and compose ASTModule
 * @param Module
 * @return true on Success or false on Error
 */
ASTModule *Parser::ParseModule() {
    FLY_DEBUG_START("Parser", "ParseModule");
    Tok.startToken();
    Tok.setKind(tok::eof);

    // Prime the lexer look-ahead.
    ConsumeToken();

    Module = Builder.CreateModule(Input.getFileName());

    // Start with Parse (recursively))
	bool Success = true;
    while (Success && Tok.isNot(tok::eof)) {
	    // Parse namespace, imports and top definitions
    	Success = ParseNameSpace() != nullptr || ParseImport() != nullptr || ParseDefinition() != nullptr;
    }

    return Module;
}

ASTModule *Parser::ParseHeader() {
    FLY_DEBUG_START("Parser", "ParseHeader");
    Tok.startToken();
    Tok.setKind(tok::eof);

    // Prime the lexer look-ahead.
    ConsumeToken();

    // Parse NameSpace on first
//    if (ParseNameSpace()) {
//        Module = Builder.CreateHeaderModule(Input.getFileName());
//
//        // Parse Top declarations
//        if (Tok.is(tok::eof)) {
//            // Node empty
//            Diag(diag::warn_empty_code);
//        } else {
//            // Parse top definitions
//            do {
//                ParseTopDef();
//            } while (ContinueParsing && Tok.isNot(tok::eof));
//        }
//    }

    return Module;
}

bool Parser::isSuccess() {
    return !Diags.hasErrorOccurred();
}

llvm::SmallVector<llvm::StringRef, 4> Parser::ParseNames() {
	llvm::SmallVector<llvm::StringRef, 4> Names;
	Names.push_back(Tok.getIdentifierInfo()->getName());
	ConsumeToken();
	do {
		//Check if is a period
		if (Tok.is(tok::period)) {
			ConsumeToken();
		}

		// Check if is an Identifier
		if (!Tok.isAnyIdentifier()) {
			Diag(Tok, diag::err_parse_identifier_expected);
			return Names;
		}

		// Parse Identifier
		Names.push_back(Tok.getIdentifierInfo()->getName());
		ConsumeToken();
	} while (Tok.is(tok::period));

	return Names;
}

ASTNameSpace *Parser::ParseNameSpace() {
	FLY_DEBUG_START("Parser", "ParseNameSpace");

	if (Tok.is(tok::kw_namespace)) {
		const SourceLocation &Loc = ConsumeToken();

		if (!Tok.isAnyIdentifier()) {
			Diag(Tok, diag::err_parse_identifier_expected);
			return nullptr;
		}

		// Create the Vector NameSpace structure
		SmallVector<llvm::StringRef, 4> Names = ParseNames();

		// Create NameSpace
		return Builder.CreateNameSpace(Loc, Names, Module);
	}
	return nullptr;
}

ASTImport *Parser::ParseImport() {
	FLY_DEBUG_START("Parser", "ParseImport");

	if (Tok.is(tok::kw_import)) {
		const SourceLocation &Loc = ConsumeToken();

		if (!Tok.isAnyIdentifier()) {
			Diag(Tok, diag::err_parse_identifier_expected);
			return nullptr;
		}

		// Parse Import identifier
		llvm::SmallVector<llvm::StringRef, 4> Names = ParseNames();

		ASTAlias *Alias = nullptr;
		if (Tok.is(tok::kw_as)) {
			ConsumeToken();

			if (!Tok.isAnyIdentifier()) {
				Diag(Tok, diag::err_parse_identifier_expected);
			} else {

				// Parse Alias identifier
				llvm::StringRef Name = Tok.getIdentifierInfo()->getName();
				const SourceLocation &AliasLoc = ConsumeToken();

				// Create Alias
				Alias = Builder.CreateAlias(AliasLoc, Name);
			}
		}

		// Create Import
		return Builder.CreateImport(Module, Loc, Names, Alias);
	}

	return nullptr;
}

ASTBase *Parser::ParseDefinition() {
	FLY_DEBUG_START("Parser", "ParseDefinition");
    assert(!isTokenComment() && "Token comment not expected");

	if (isTokenComment()) {
		return ParseComment();
	}

	// Parse Top Scopes: Public/Private and Constant
	SmallVector<ASTScope *, 8> Scopes = ParseScopes();

    // Define a Class
    if (Tok.isOneOf(tok::kw_struct, tok::kw_class, tok::kw_interface)) {
        return ParseClass(Scopes);
    }

    if (Tok.is(tok::kw_enum)) {
        return ParseEnum(Scopes);
    }

    // Parse Type
    ASTTypeRef *TypeRef = ParseTypeRef();
    if (TypeRef == nullptr) {
        Diag(Tok.getLocation(), diag::err_parser_invalid_type);
    } else {
        if (Tok.isAnyIdentifier()) {

            // Define a Function
            if (Lexer::findNextToken(Tok.getLocation(), SourceMgr)->is(tok::l_paren)) {
                return ParseFunction(Scopes, TypeRef);
            } else {
                // Define a GlobalVar
                return ParseGlobalVar(Scopes, TypeRef);
            }
        } else {
            Diag(Tok, diag::err_parse_identifier_invalid);
        }
    }

    return nullptr;
}

SmallVector<ASTScope *, 8> Parser::ParseScopes() {
    FLY_DEBUG_START("ClassParser", "ParseScopes");

    SemaBuilderScopes *BuilderScopes = SemaBuilderScopes::Create();

    while (Tok.isNot(tok::eof)) {
        ASTScope *Scope;
        if (Tok.is(tok::kw_private)) {
            BuilderScopes->addVisibility(ConsumeToken(), ASTVisibilityKind::V_PRIVATE);
        } else if (Tok.is(tok::kw_protected)) {
            BuilderScopes->addVisibility(ConsumeToken(), ASTVisibilityKind::V_PROTECTED);
        } else if (Tok.is(tok::kw_public)) {
            BuilderScopes->addVisibility(ConsumeToken(), ASTVisibilityKind::V_PUBLIC);
        } else if (Tok.is(tok::kw_const)) {
            BuilderScopes->addConstant(ConsumeToken(), true);
        } else if (Tok.is(tok::kw_static)) {
            BuilderScopes->addStatic(ConsumeToken(), true);
        } else {
            break;
        }
    }

    return BuilderScopes->getScopes();
}

/**
 * ParseModule Global Var declaration
 * @param Visibility
 * @param Constant
 * @param TypeRef
 * @param Name
 * @param NameLoc
 * @return
 */
ASTVar *Parser::ParseGlobalVar(SmallVector<ASTScope *, 8> &Scopes, ASTTypeRef *TypeRef) {
	FLY_DEBUG_START("Parser", "ParseGlobalVar");
    assert(Tok.isAnyIdentifier() && "Tok must be an Identifier");

    llvm::StringRef Name = Tok.getIdentifierInfo()->getName();
    SourceLocation Loc = ConsumeToken();

    // Parsing =
    ASTExpr *Expr = nullptr;
    if (isAssignOperator(Tok)) {
        ConsumeToken();

        // Parse Expr
        Expr = ParseExpr();
    }

    // GlobalVar
    ASTVar *GlobalVar = Builder.CreateGlobalVar(Module, Loc, TypeRef, Name, Scopes, Expr);

    return GlobalVar;
}


/**
 * ParseModule Function declaration
 * @param Visibility
 * @param Constant
 * @param TypeRef
 * @param Name
 * @param NameLoc
 * @return
 */
ASTFunction *Parser::ParseFunction(SmallVector<ASTScope *, 8> &Scopes, ASTTypeRef *TypeRef) {
	FLY_DEBUG_START("Parser", "ParseFunction");

	StringRef Name = Tok.getIdentifierInfo()->getName();
	const SourceLocation &Loc = ConsumeToken();
	SmallVector<ASTVar *, 8> Params = ParserFunction::ParseParams(this);
	ASTFunction *Function = Builder.CreateFunction(Module, Loc, TypeRef, Name, Scopes, Params, nullptr);
	ASTBlockStmt *Body = isBlockStart() ? ParserFunction::ParseBody(this, Function) : nullptr;
	return Function;
}

/**
 * ParseModule Class declaration
 * @param Visibility
 * @param Constant
 * @return
 */
ASTClass * Parser::ParseClass(SmallVector<ASTScope *, 8> &Scopes) {
	FLY_DEBUG_START("Parser", "ParseClass");

    ASTClass *Class = ParserClass::Parse(this, Scopes);
    return Class;
}


/**
 * ParseModule Enum declaration
 * @param Visibility
 * @param Constant
 * @return
 */
ASTEnum *Parser::ParseEnum(SmallVector<ASTScope *, 8>&Scopes) {
	FLY_DEBUG_START("Parser", "ParseEnum");
    assert(Tok.is(tok::kw_enum) && "Token Enum expected");

    ASTEnum *Enum = ParserEnum::Parse(this, Scopes);
    return Enum;
}

/**
 * Parse a Comment
 * @return the Comment
 */
ASTComment *Parser::ParseComment() {
	FLY_DEBUG_START("Parser", "ParseComment");
	assert(!Lex.BlockComment.empty() && "Block Comment must be not empty");

	// Parse all as string
	ASTComment *Comment = nullptr;
	if (!Lex.BlockComment.empty()) {
		Comment = Builder.CreateComment(Module, SourceLocation(), Lex.BlockComment);
	}

	return Comment;
}

void Parser::ParseBlockOrStmt(ASTBlockStmt* Parent) {
	FLY_DEBUG_START("Parser", "ParseBlockOrStmt");
    isBlockStart() ?
        ParseBlock(Parent) : // parse more than one Stmt
        ParseStmt(Parent); // parse a single Stmt
}

/**
 * ParseModule statements between braces
 * @param Stmt
 * @return
 */
void Parser::ParseBlock(ASTBlockStmt *Block) {
	FLY_DEBUG_START("Parser", "ParseBlock");
    assert(isBlockStart() && "Block Start");

	// Consume '{'
    ConsumeBrace(BracketCount);

    while (true) {

    	// End of File -> end of parsing
    	if (Tok.is(tok::eof)) {
    		ConsumeToken();
    		return;
    	}

    	// Check if block is balanced
    	if (isBlockEnd()) {
    		if (not isBraceBalanced()) {
    			Diag(Tok.getLocation(), diag::err_parser_unclosed_bracket);
    		}

    		Lex.ClearBlockComment();

    		// Consume '}'
    		ConsumeBrace(BracketCount);
    		return;
    	}

    	ParseStmt(Block);
    }
}

/**
 * ParseModule a single statement like Variable declaration, assignment or Function invocation
 *
 * cont int a
 * a = ...
 * a()
 * a v1 = ...
 * int a = ...
 *
 * @param Parent
 * @return true on Success or false on Error
 */
void Parser::ParseStmt(ASTBlockStmt *Parent) {
	FLY_DEBUG_START("Parser", "ParseStmt");

    // Parse if stmt
    if (Tok.is(tok::kw_if)) {
        return ParseIfStmt(Parent);
    }

    // Parse switch stmt
    if (Tok.is(tok::kw_switch)) {
        return ParseSwitchStmt(Parent);
    }

    // Parse for stmt
    if (Tok.is(tok::kw_for)) {
        return ParseForStmt(Parent);
    }

    // Parse while stmt
    if (Tok.is(tok::kw_while)) {
        return ParseWhileStmt(Parent);
    }

    // Parse handle stmt
    if (Tok.is(tok::kw_handle)) {
        return ParseHandleStmt(Parent, nullptr);
    }

    // Parse fail stmt
    if (Tok.is(tok::kw_fail)) {
        return ParseFailStmt(Parent);
    }

    // Parse return stmt
    if (Tok.is(tok::kw_return)) { // Parse return
        SourceLocation Loc = ConsumeToken();
        SemaBuilderStmt *BuilderStmt = Builder.CreateReturnStmt(Parent, Loc);
        ASTExpr *Expr = ParseExpr();
        BuilderStmt->setExpr(Expr);
        return;
    }

    // Parse break stmt
    if (Tok.is(tok::kw_break)) { // Parse break
        ASTBreakStmt *Break = Builder.CreateBreakStmt(Parent, ConsumeToken());
        return;
    }

    // Parse continue stmt
    if (Tok.is(tok::kw_continue)) { // Parse continue
        ASTContinueStmt *Continue = Builder.CreateContinueStmt(Parent, ConsumeToken());
        return;
    }

    // Parse scopes
    SmallVector<ASTScope *, 8> Scopes = ParseScopes();

	// Parse a Local Var
	Optional<Token> NexTok = Tok;
	Optional<Token> NexTok2 = Tok;
    if (isVarOrType(NexTok) && isVarOrType(NexTok) || isBuiltinType(Tok) && isVarOrType(NexTok2)) {

        // const int a
        // Type a
        // int a = ...
        // Type a = ...
        ASTTypeRef *TypeRef = ParseTypeRef();
        if (TypeRef == nullptr) { // FIXME need to be removed in place of master Validator
            Diag(Tok.getLocation(), diag::err_parser_invalid_type);
            return;
        }

        // Create a Local Var
    	llvm::StringRef Name = Tok.getIdentifierInfo()->getName();
    	const SourceLocation &Loc = ConsumeToken();
        ASTVar *LocalVar = Builder.CreateLocalVar(Parent, Loc, TypeRef, Name, Scopes);

        // Assign to LocalVar
        if (isAssignOperator(Tok)) {
            ConsumeToken();

            if (Tok.is(tok::kw_handle)) {
                ASTVarRef *ErrorVarRef = Builder.CreateVarRef(LocalVar);
                ParseHandleStmt(Parent, ErrorVarRef);
            } else {
                SemaBuilderStmt *BuilderStmt = Builder.CreateAssignmentStmt(Parent, LocalVar);

                // Parse Expr
                ASTExpr *Expr = ParseExpr();
                BuilderStmt->setExpr(Expr);
            }
        }

        return;
    }

    // Parse a Var Assignment
    if (isVarOrType(NexTok) && isAssignOperator(NexTok.getValue())) {
        // Parse Var
        // a = ...
        ASTVarRef *VarRef = ParseVarRef();

        // Create Left Expr only for +=, -=, *=, etc.
        ASTExpr *Left = nullptr;
        if (Tok.is(tok::equal)) {
            ConsumeToken();
        } else {
            Left = Builder.CreateExpr(VarRef);
        }

        // Parse Expr
        ASTExpr *Expr = ParserExpr::Parse(this, Left);
        SemaBuilderStmt *BuilderStmt = Builder.CreateAssignmentStmt(Parent, VarRef);
        BuilderStmt->setExpr(Expr);
        return;
    }

    // a()
    // a++
    // ++a
    // new A()
    ASTExpr *Expr = ParseExpr();
    SemaBuilderStmt *BuilderStmt = Builder.CreateExprStmt(Parent, Expr->getLocation());
    BuilderStmt->setExpr(Expr);
}

bool Parser::isVarOrType(Optional<Token> &NexTok) {
	do {
		if (NexTok.getValue().isAnyIdentifier()) {
			NexTok = Lexer::findNextToken(NexTok->getLocation(), SourceMgr);
		} else {
			return false;
		}
	} while (NexTok.getValue().is(tok::period) && ((NexTok = Lexer::findNextToken(Tok.getLocation(), SourceMgr))));

	if (NexTok->is(tok::l_paren)) {
		return false;
	}


	if (NexTok->isOneOf(tok::plusplus, tok::minusminus)) {
		return false;
	}

	return true;
}

/**
 * ParseModule open paren ( at start of cycle into condition statements
 * @return true on Success or false on Error
 */
bool Parser::ParseStartParen() {
    FLY_DEBUG_START("Parser", "ParseStartParen");

    bool HasParen = false;
    if (Tok.is(tok::l_paren)) {
        ConsumeParen();
        HasParen = true;
    }
    return HasParen;
}

/**
 * ParseModule close paren ) at end of cycle into condition statements
 * @return true on Success or false on Error
 */
void Parser::ParseEndParen(bool HasParen) {
    FLY_DEBUG_MESSAGE("Parser", "ParseStartParen", "HasParen=" << HasParen);

    if (HasParen) {
        if (Tok.is(tok::r_paren)) {
            ConsumeParen();
        } else {
            Diag(diag::err_right_paren);
        }
    }
}

/**
 * ParseModule If, Elseif and Else statements
 *
 *  if ... {
 *    ...
 *  } elsif ... {
 *    ...
 *  } else {
 *    ...
 *  }
 *
 *  or
 *
 *  if (...) ...
 *  elsif (...) ...
 *  else ...
 *
 * @param Parent
 * @return true on Success or false on Error
 */
void Parser::ParseIfStmt(ASTBlockStmt *Parent) {
	FLY_DEBUG_START("Parser", "ParseIfStmt");
    assert(Tok.is(tok::kw_if) && "Token is if keyword");
    ConsumeToken();

    // Parse (
    bool hasParen = ParseStartParen();

    // Parse the group of expressions into parenthesis
    ASTExpr *IfCondition = ParseExpr(); // Parse Expr

	// Parse )
    if (hasParen) {
        ParseEndParen(hasParen);
    }

    // Create If
    SemaBuilderIfStmt *IfBuilder = Builder.CreateIfBuilder(Parent);
    ASTBlockStmt *IfBlock = Builder.CreateBlockStmt(Tok.getLocation());
    IfBuilder->If(Tok.getLocation(), IfCondition, IfBlock);

    // Parse statement between braces for If
    ParseBlockOrStmt(IfBlock);

    // Add Elsif
    while (Tok.is(tok::kw_elsif)) {
        const SourceLocation &ElsifLoc = ConsumeToken();

        // Parse (
        bool hasElsifParen = ParseStartParen();
        // Parse the group of expressions into parenthesis
        ASTExpr *ElsifCondition = ParseExpr(); // Parse Expr
        if (hasElsifParen) {
            ParseEndParen(hasElsifParen);
        }

        ASTBlockStmt *ElsifBlock = Builder.CreateBlockStmt(Tok.getLocation());
        ParseBlockOrStmt(ElsifBlock);

        IfBuilder->ElseIf(ElsifLoc, ElsifCondition, ElsifBlock);
    }

    // Add Else
    if (Tok.is(tok::kw_else)) {
        const SourceLocation &ElseLoc = ConsumeToken();
        ASTBlockStmt *ElseBlock = Builder.CreateBlockStmt(ElseLoc);
        ParseBlockOrStmt(ElseBlock);
    	IfBuilder->Else(ElseLoc, ElseBlock);
    }
}

/**
 * ParseModule the Switch statement
 *
 * switch var {
 *  case 1:
 *      ...
 *      break
 *  case 2 {
 *      ...
 *      ...
 *      }
 *      break
 *  case 3:
 *      break
 *  case 4
 *  default
 *      ...
 * }
 *
 * @param Parent
 * @return true on Success or false on Error
 */
void Parser::ParseSwitchStmt(ASTBlockStmt *Parent) {
	FLY_DEBUG_START("Parser", "ParseSwitchStmt");
    assert(Tok.is(tok::kw_switch) && "Token is switch keyword");
	const SourceLocation &Loc = ConsumeToken();
    
    // Parse (
    bool hasParen = ParseStartParen();
    
    // Parse switch keyword
    // Parse Var reference like (a)
    ASTExpr *Expr = ParseExpr();

	// Parse )
	if (hasParen) {
		ParseEndParen(hasParen);
	}

	// Create Switch
	SemaBuilderSwitchStmt *SwitchBuilder = Builder.CreateSwitchBuilder(Parent);
    SwitchBuilder->Switch(Loc, Expr);

	// TODO implement duplicity check of 'default'

    // Init Switch Statement and start parse from brace
    if (isBlockStart()) {
		ConsumeBrace(BracketCount);

    	while (true) {
    		if (Tok.is(tok::kw_case)) {
    			ConsumeToken();

    			// Parse Expression for different cases
    			// for a Value  -> case 1:
    			// for a Var -> case a:
    			// for a default
    			ASTExpr *Expr = ParseExpr();

    			// Parse Switch
    			ASTBlockStmt *CaseBlock = Builder.CreateBlockStmt(Tok.getLocation());
    			if (Tok.is(tok::colon)) { // Parse a Block of Stmt
    				ConsumeToken();
    				SwitchBuilder->Case(Loc, Expr, CaseBlock);
    				if (Tok.isOneOf(tok::kw_case, tok::kw_default)) {
    					continue;
    				}
    				ParseBlockOrStmt(CaseBlock);
    			}
    		} else if (Tok.is(tok::kw_default)) {
    			ConsumeToken();

    			if (SwitchBuilder->hasDefault()) {
    				Diag(Tok.getLocation(), diag::err_syntax_default_error);
    				return;
    			}

    			// Parse Default
    			ASTBlockStmt *DefaultBlock = Builder.CreateBlockStmt(Tok.getLocation());
    			if (Tok.is(tok::colon)) { // Parse a Block of Stmt
    				ConsumeToken();
    				SwitchBuilder->Default(Loc, DefaultBlock);
    				if (Tok.is(tok::kw_case)) {
    					continue;
    				}
    				ParseBlockOrStmt(DefaultBlock);
    			}
    		} else {
    			break;
    		}
    	}

        // Switch statement is at end of it's time add current Switch to parent statement
        if (isBlockEnd()) {
            ConsumeBrace(BracketCount);
        	return;
        }
    }

    Diag(diag::err_syntax_error);
}

/**
 * ParseModule the While statement
 *
 * while (...) {
 *  ...
 * }
 *
 * or
 *
 * while ... {
 *  ...
 * }
 *
 * @param Parent
 * @return true on Success or false on Error
 */
void Parser::ParseWhileStmt(ASTBlockStmt *Parent) {
	FLY_DEBUG_START("Parser", "ParseWhileParen");
    assert(Tok.is(tok::kw_while) && "Token is while keyword");

    const SourceLocation &Loc = ConsumeToken();

    // Consume Left Parenthesis ( if exists
    bool hasParen = ParseStartParen();

	// Create AST While Block
	ASTExpr *Condition = ParseExpr();

	// Parse )
	if (hasParen) {
		ParseEndParen(hasParen);
	}

    ASTBlockStmt *BlockStmt = Builder.CreateBlockStmt(Tok.getLocation());
    SemaBuilderLoopStmt *LoopBuilder = Builder.CreateLoopBuilder(Parent, Loc);
    LoopBuilder->Loop(Condition, BlockStmt);

    // Parse statement between braces
    ParseBlockOrStmt(BlockStmt);
}

/**
 * ParseModule the For statement
 *
 * for ...; ...; ... {
 *  ...
 * }
 *
 * or
 *
 * for (...; ...; ...) {
 *  ...
 * }
 *
 * @param Parent
 * @return true on Success or false on Error
 */
void Parser::ParseForStmt(ASTBlockStmt *Parent) {
	FLY_DEBUG_START("Parser", "ParseForStmt");
    assert(Tok.is(tok::kw_for) && "Token is for keyword");

    const SourceLocation &Loc = ConsumeToken();

    // Consume Left Parenthesis ( if exists
    bool hasParen = ParseStartParen();

    // Create For Statement
	SemaBuilderLoopStmt *LoopBuilder = Builder.CreateLoopBuilder(Parent, Loc);
    ASTBlockStmt *InitBlock = Builder.CreateBlockStmt(Tok.getLocation());
	LoopBuilder->Init(InitBlock);
    ASTExpr *Condition = nullptr;

    // for int a = 1, b = 2; i < 0; i++

	ParseStmt(InitBlock);
	while (Tok.is(tok::comma)) {
		ConsumeToken();
		ParseStmt(InitBlock);
	}

    // This is an Expression, it could be a Condition
    if (Tok.is(tok::semi)) {
        ConsumeToken();

        Condition = ParseExpr();

        if (Tok.is(tok::semi)) {
        	ASTBlockStmt *PostBlock = Builder.CreateBlockStmt(ConsumeToken());
        	LoopBuilder->Post(PostBlock);
            ParseStmt(PostBlock);
        	while (Tok.is(tok::comma)) {
        		ConsumeToken();
        		ParseStmt(PostBlock);
        	}
        }
    }

	// Parse )
	if (hasParen) {
		ParseEndParen(hasParen);
	}

	// Create Loop Stmt
    ASTBlockStmt *LoopBlock = Builder.CreateBlockStmt(Tok.getLocation());
    LoopBuilder->Loop(Condition, LoopBlock);


    // Parse statement between braces
    ParseBlockOrStmt(LoopBlock);
}

void Parser::ParseHandleStmt(ASTBlockStmt *Parent, ASTVarRef *Error) {
	FLY_DEBUG_START("Parser", "ParseHandleStmt");
    assert(Tok.is(tok::kw_handle) && "Token is handle keyword");

    // Consume handle keyword
    const SourceLocation &Loc = ConsumeToken();

    // Parse statement between braces
    ASTBlockStmt *HandleBlock = Builder.CreateBlockStmt(Loc);
    Builder.CreateHandleStmt(Parent, Loc, HandleBlock, Error);

    ParseBlockOrStmt(HandleBlock);
}

void Parser::ParseFailStmt(ASTBlockStmt *Parent) {
    FLY_DEBUG_START("Parser", "ParseFailStmt");
    assert(Tok.is(tok::kw_fail) && "Token is handle keyword");

    const SourceLocation &Loc = ConsumeToken();
    SemaBuilderStmt *BuilderStmt = Builder.CreateFailStmt(Parent, Loc);
    ASTExpr *Expr = ParseExpr();
    BuilderStmt->setExpr(Expr);
}

/**
 * ParseModule a data Type
 * @return true on Success or false on Error
 */
ASTTypeRef *Parser::ParseTypeRef() {
    FLY_DEBUG_START("Parser", "ParseTypeRef");

    ASTTypeRef *TypeRef = nullptr;
    if (isBuiltinType(Tok)) {
    	return ParseBuiltinTypeRef();
    }

	// Parse Class or Enum Type
	if (Tok.isAnyIdentifier()) {
		llvm::SmallVector<llvm::StringRef, 4> Names;
		Names.push_back(Tok.getIdentifierInfo()->getName());
		const SourceLocation &Loc = ConsumeToken();
		do {

			//Check if is a period
			if (Tok.is(tok::period)) {
				ConsumeToken();
			}

			// Check if is an Identifier
			if (!Tok.isAnyIdentifier()) {
				Diag(Tok, diag::err_parse_identifier_expected);
				return TypeRef;
			}

			// Parse Identifier
			llvm::StringRef Name = Tok.getIdentifierInfo()->getName();
			const SourceLocation &NameLoc = ConsumeToken();

			// with Array Type al previous Names are NameSpace
			if (isArrayType(Tok)) {
				TypeRef = Builder.CreateTypeRef(NameLoc, Name, Builder.CreateNameSpaceRef(Loc, Names));
				ASTArrayTypeRef *ArrayTypeRef = ParseArrayTypeRef(TypeRef);
				return ArrayTypeRef;
			}

			// Add Name, but you don't know if is a Type or a NameSpace
			Names.push_back(Name);

		} while (Tok.is(tok::period));


		if (Names.empty()) {
			Diag(Tok.getLocation(), diag::err_syntax_error);
			return nullptr;
		}

		// Take last Name as TypeRef
		llvm::StringRef TypeRefName = Names[Names.size()-1];

		// Take all Names as NameSpace
		llvm::SmallVector<llvm::StringRef, 4> NameSpace;
		if (Names.size() > 1) {
			for (size_t i = Names.size()-2; i > 0; --i) {
				NameSpace.push_back(Names[i]);
			}
		}

		// Create TypeRef with parent namespace
		TypeRef = Builder.CreateTypeRef(Loc, TypeRefName, Builder.CreateNameSpaceRef(Loc, NameSpace));
	}

    return TypeRef;
}

ASTTypeRef * Parser::ParseBuiltinTypeRef() {
	ASTTypeRef *TypeRef = nullptr;
	switch (Tok.getKind()) {
	case tok::kw_bool:
		TypeRef = Builder.CreateBoolTypeRef(ConsumeToken());
		break;
	case tok::kw_byte:
		TypeRef = Builder.CreateByteTypeRef(ConsumeToken());
		break;
	case tok::kw_ushort:
		TypeRef = Builder.CreateUShortTypeRef(ConsumeToken());
		break;
	case tok::kw_short:
		TypeRef = Builder.CreateShortTypeRef(ConsumeToken());
		break;
	case tok::kw_uint:
		TypeRef = Builder.CreateUIntTypeRef(ConsumeToken());
		break;
	case tok::kw_int:
		TypeRef = Builder.CreateIntTypeRef(ConsumeToken());
		break;
	case tok::kw_ulong:
		TypeRef = Builder.CreateULongTypeRef(ConsumeToken());
		break;
	case tok::kw_long:
		TypeRef = Builder.CreateLongTypeRef(ConsumeToken());
		break;
	case tok::kw_float:
		TypeRef = Builder.CreateFloatTypeRef(ConsumeToken());
		break;
	case tok::kw_double:
		TypeRef = Builder.CreateDoubleTypeRef(ConsumeToken());
		break;
	case tok::kw_void:
		TypeRef = Builder.CreateVoidTypeRef(ConsumeToken());
		break;
	case tok::kw_char:
		TypeRef = Builder.CreateCharTypeRef(ConsumeToken());
		break;
	case tok::kw_string:
		TypeRef = Builder.CreateStringTypeRef(ConsumeToken());
		break;
	case tok::kw_error:
		TypeRef = Builder.CreateErrorTypeRef(ConsumeToken());
		break;
	default:
		Diag(Tok.getLocation(), diag::err_parser_invalid_type);
		TypeRef = nullptr;
	}
	return isArrayType(Tok) ? ParseArrayTypeRef(TypeRef) : TypeRef;
}

ASTArrayTypeRef *Parser::ParseArrayTypeRef(ASTTypeRef *TypeRef) {
	FLY_DEBUG_START("Parser", "ParseArrayType");
	assert(isArrayType(Tok) && "Invalid array parse");

	//TODO array multidimensional

	ASTArrayTypeRef *ArrayTypeRef = nullptr;
	const SourceLocation &Loc = ConsumeBracket();
	ASTExpr *Expr = ParseExpr();
	if (Tok.is(tok::r_square)) {
		ConsumeBracket();
		ArrayTypeRef = Builder.CreateArrayTypeRef(Loc, TypeRef, Expr);
	} else {
		Diag(Loc, diag::err_parser_unclosed_bracket);
	}

	return ArrayTypeRef;
}

ASTVarRef * Parser::ParseVarRef() {
	FLY_DEBUG_START("Parser", "ParseVarRef");
	assert(Tok.isAnyIdentifier() && "VarRef start with Name");

	// Parse NameSpace or Class/Enum
	ASTRef *Ref = ParseRef();

	if (Ref->isCall()) {
		// Error: unexpected Call
		Diag(Ref->getLocation(), diag::err_syntax_error);
		return nullptr;
	}

	ASTVarRef * VarRef = Builder.CreateVarRef(Ref);

	FLY_DEBUG_END("Parser", "ParseVarRef");
	return VarRef;
}

/**
 * ParseModule a Function Call
 * @return true on Success or false on Error
 */
ASTCall *Parser::ParseCall() {
	llvm::StringRef Name = Tok.getIdentifierInfo()->getName();
	const SourceLocation &Loc = ConsumeToken();
	return ParseCall(Loc, Name, nullptr);
}

/**
 * ParseModule a Function Call
 * @param Block
 * @param Id
 * @param Loc
 * @return true on Success or false on Error
 */
ASTCall *Parser::ParseCall(const SourceLocation &Loc, llvm::StringRef Name, ASTRef *Parent) {
	FLY_DEBUG_START("Parser", "ParseCall");
    assert(Tok.is(tok::l_paren) && "Call start with parenthesis");

    // Parse Call args
    ConsumeParen(); // consume l_paren

    // Parse Args in a Function Call
    llvm::SmallVector<ASTExpr *, 8> Args;
    while (true) {
        // Check for closing parenthesis (end of parameter list)
        if (Tok.is(tok::r_paren)) {
            ConsumeParen();
            break;
        }

        // Parse a parameter
        ASTExpr *Arg = ParseExpr();
        if (Arg == nullptr) {
            // Handle error: Invalid parameter syntax
            Diag(Tok.getLocation(), diag::err_parser_invalid_param);
            break;
        }

        // Add the parsed parameter to the list
        Args.push_back(Arg);

        // Check for a comma (',') to separate parameters
        if (Tok.is(tok::comma)) {
            ConsumeToken(); // Consume the comma and continue
        } else if (Tok.is(tok::r_paren)) {
            ConsumeParen();
            break; // End of parameter list
        } else {
            // Handle error: Unexpected token
            Diag(Tok.getLocation(), diag::err_parse_expected_comma_or_rparen);
        }
    }

    return Builder.CreateCall(Loc, Name, Args, ASTCallKind::CALL_FUNCTION, Parent);
}

ASTRef * Parser::ParseCallOrVarRef() {
	ASTRef * Ref = ParseRef();
	if (!Ref->isCall()) {
		return Builder.CreateVarRef(Ref);
	}
	return Ref;
}

/**
 * ParseModule as Identifier
 * @return
 */
ASTRef *Parser::ParseRef(ASTRef *Parent) {
    FLY_DEBUG_START("Parser", "ParseIdentifier");
    assert(Tok.isAnyIdentifier() && "Token Identifier expected");

	ASTRef *Ref = nullptr;
	do {
		llvm::StringRef Name = Tok.getIdentifierInfo()->getName();
		const SourceLocation &Loc = ConsumeToken();

		if (Tok.is(tok::l_paren)) {
			Ref = ParseCall(Loc, Name, Parent);
		} else {
			Ref = Builder.CreateUndefinedRef(Loc, Name, Parent);
		}
		Parent = Ref;
	} while (Tok.is(tok::period) && ConsumeToken().isValid());

    return Ref;
}

/**
 * ParseModule a Value Expression
 * @return the ASTValueExpr
 */
ASTValue *Parser::ParseValue() {
    FLY_DEBUG_START("Parser", "ParseValue");

    if (Tok.is(tok::kw_null)) {
        const SourceLocation &Loc = ConsumeToken();
        return Builder.CreateNullValue(Loc);
    }

    // Parse Numeric Constants
    if (Tok.is(tok::numeric_constant)) {
        llvm::StringRef Val = llvm::StringRef(Tok.getLiteralData(), Tok.getLength());
        return ParseValueNumber(Val);
    }

    if (Tok.isCharLiteral()) {
        llvm::StringRef Val = llvm::StringRef(Tok.getLiteralData(), Tok.getLength());
        return Builder.CreateCharValue(ConsumeToken(), Val);
    }

    if (Tok.isStringLiteral()) {
        llvm::StringRef Val = llvm::StringRef(Tok.getLiteralData(), Tok.getLength());
        return Builder.CreateStringValue(ConsumeStringToken(), Val);
    }

    // Parse true or false boolean values
    if (Tok.is(tok::kw_true)) {
        return Builder.CreateBoolValue(ConsumeToken(), true);
    }
    if (Tok.is(tok::kw_false)) {
        return Builder.CreateBoolValue(ConsumeToken(), false);
    }

    // Parse Array or Struct
    if (Tok.is(tok::l_brace)) {
        return ParseValues();
    }

    Diag(diag::err_invalid_value) << Tok.getName();
    return nullptr;
}

ASTValue *Parser::ParseValueNumber(llvm::StringRef Value) {
    FLY_DEBUG_MESSAGE("Parser", "ParseValueNumber", "Value=" << Value);

    const SourceLocation &Loc = ConsumeToken();

    llvm::Regex FloatRegex(R"(^[-+]?[0-9]*\.[0-9]+([eE][-+]?[0-9]+)?$)");

    if (Value.substr(0, 2) == "0b" || Value.substr(0, 2) == "0B") {
        // Binary
        return Builder.CreateIntegerValue(Loc, Value, 2);
    } else if (Value.substr(0, 2) == "0x" || Value.substr(0, 2) == "0X") {
        // Hexadecimal
        return Builder.CreateIntegerValue(Loc, Value, 16);
    } else if (Value[0] == '0' && Value.size() > 1) {
        // Octal
        return Builder.CreateIntegerValue(Loc, Value, 8);
    } else if (FloatRegex.match(Value)) {
        // Floating point
        return Builder.CreateFloatingValue(Loc, Value);
    } else {
        // Decimal
        return Builder.CreateIntegerValue(Loc, Value, 10);
    }

}

/**
 * ParseModule Array Value Expression
 * @return the ASTValueExpr
 */
ASTValue *Parser::ParseValues() {
    FLY_DEBUG_START("Parser", "ParseValues");
    const SourceLocation &StartLoc = ConsumeBrace(BracketCount);
    
    // Set Values Struct and Array for next
    bool isStruct = false;
    llvm::StringMap<ASTValue *> StructValues;
    llvm::SmallVector<ASTValue *, 8> ArrayValues;

    // Parse array values Ex. {1, 2, 3}
    while(Tok.isNot(tok::r_brace)) {

        // if is Identifier -> struct
        if (Tok.isAnyIdentifier()) {
            isStruct = true;
            const StringRef &Key = Tok.getIdentifierInfo()->getName();
            ConsumeToken();
            
            if (Tok.is(tok::equal)) {
                // FIXME
                ConsumeToken();

                ASTValue *Value = ParseValue();
                if (Value) {
                    StructValues.insert(std::make_pair(Key, Value));
                } else {
                    Diag(diag::err_invalid_value) << Tok.getName();
                }
            }
        } else { // if is Value -> array
            ASTValue *Value = ParseValue();
            if (Value) {
                ArrayValues.push_back(Value);
            } else {
                Diag(diag::err_invalid_value) << Tok.getName();
            }
        }

        if (Tok.is(tok::comma)) {
            ConsumeToken();
        } else {
            break;
        }
    };

    // End of Array
    if (Tok.is(tok::r_brace)) {
        ConsumeBrace(BracketCount);
        if (isStruct) {
            return Builder.CreateStructValue(StartLoc, StructValues);
        } else {
            return Builder.CreateArrayValue(StartLoc, ArrayValues);
        }
    }

    Diag(diag::err_invalid_value) << Tok.getName();
    return Builder.CreateZeroValue(Tok.getLocation());
}

ASTExpr *Parser::ParseExpr() {
    FLY_DEBUG_START("Parser", "ParseExpr");
    return ParserExpr::Parse(this);
}

bool Parser::isBuiltinType(Token &Tok) {
    FLY_DEBUG_START("Parser", "isBuiltinType");
    return Tok.isOneOf(tok::kw_bool, tok::kw_byte, tok::kw_ushort, tok::kw_short, tok::kw_uint, tok::kw_int,
                       tok::kw_ulong, tok::kw_long, tok::kw_float, tok::kw_double, tok::kw_void, tok::kw_string,
                       tok::kw_char, tok::kw_error);
}

bool Parser::isArrayType(Token &Tok) {
    FLY_DEBUG_START("Parser", "isArrayType");
    return Tok.is(tok::l_square);
}

/**
 * Check if Token is a Value
 * @return true on Success or false on Error
 */
bool Parser::isValue() {
    FLY_DEBUG_START("Parser", "isValue");
    return Tok.isOneOf(tok::numeric_constant, tok::kw_true, tok::kw_false, tok::kw_null, tok::l_brace,
                       tok::char_constant, tok::string_literal);
}

/**
 * ParseModule const scope of vars and class
 * @param Constant
 * @return true on Success or false on Error
 */
bool Parser::isConst() {
    FLY_DEBUG_START("Parser", "isConst");
    if (Tok.is(tok::kw_const)) {
        ConsumeToken();
        return true;
    }
    return false;
}

bool Parser::isBlockStart() {
    FLY_DEBUG_START("Parser", "isBlockStart");
    return Tok.is(tok::l_brace);
}

bool Parser::isBlockEnd() {
    FLY_DEBUG_START("Parser", "isBlockEnd");
    return Tok.is(tok::r_brace);
}

/**
 * ConsumeToken - Consume the current 'peek token' and lex the next one.
 * This does not work with special tokens: string literals,
 * annotation tokens and balanced tokens must be handled using the specific
 * consume methods.
 * @return the location of the consumed token
 */
SourceLocation Parser::ConsumeToken() {
    FLY_DEBUG_START("Parser", "ConsumeToken");
    assert(!isTokenSpecial() && "Should consume special tokens with Consume*Token");
    PrevTokLocation = Tok.getLocation();
    Lex.Lex(Tok);
    return PrevTokLocation;
}

/**
 * isTokenParen - Return true if the cur token is '(' or ')'.
 * @return
 */
bool Parser::isTokenParen() const {
    FLY_DEBUG_START("Parser", "isTokenParen");
    return Tok.isOneOf(tok::l_paren, tok::r_paren);
}

/**
 * isTokenBracket - Return true if the cur token is '[' or ']'.
 * @return
 */
bool Parser::isTokenBracket() const {
    FLY_DEBUG_START("Parser", "isTokenBracket");
    return Tok.isOneOf(tok::l_square, tok::r_square);
}

/**
 * isTokenBrace - Return true if the cur token is '{' or '}'.
 * @return
 */
bool Parser::isTokenBrace() const {
    FLY_DEBUG_START("Parser", "isTokenBrace");
    return Tok.isOneOf(tok::l_brace, tok::r_brace);
}

/**
 * isTokenConsumeParenStringLiteral - True if this token is a string-literal.
 * @return
 */
bool Parser::isTokenStringLiteral() const {
    FLY_DEBUG_START("Parser", "isTokenStringLiteral");
    return tok::isStringLiteral(Tok.getKind());
}

/**
 * isTokenSpecial - True if this token requires special consumption methods.
 * @return
 */
bool Parser::isTokenSpecial() const {
    FLY_DEBUG_START("Parser", "isTokenSpecial");
    return isTokenStringLiteral() || isTokenParen() || isTokenBracket() ||
           isTokenBrace();
}

bool Parser::isTokenComment() const {
    FLY_DEBUG_START("Parser", "isTokenComment");
    return Tok.getKind() == tok::comment;
}

bool Parser::isAssignOperator(const Token &Tok) const {
	FLY_DEBUG_START("Parser", "isAssignOperator");
	return Tok.isOneOf(tok::equal, tok::plusequal, tok::minusequal, tok::starequal, tok::slashequal,
				   tok::percentequal, tok::ampequal, tok::pipeequal, tok::caretequal, tok::lesslessequal,
				   tok::greatergreaterequal);
}

/**
 * ConsumeParen - This consume method keeps the paren count up-to-date.
 * @return
 */
SourceLocation Parser::ConsumeParen() {
    FLY_DEBUG_START("Parser", "ConsumeParen");
    assert(isTokenParen() && "wrong consume method");
    if (Tok.getKind() == tok::l_paren)
        ++ParenCount;
    else if (ParenCount) {
        //AngleBrackets.clear(*this);
        --ParenCount;       // Don't let unbalanced )'s drive the count negative.
    }

    PrevTokLocation = Tok.getLocation();
    Lex.Lex(Tok);
    return PrevTokLocation;
}

/**
 * ConsumeBracket - This consume method keeps the bracket count up-to-date.
 * @return
 */
SourceLocation Parser::ConsumeBracket() {
    FLY_DEBUG_START("Parser", "ConsumeBracket");
    assert(isTokenBracket() && "wrong consume method");
    if (Tok.getKind() == tok::l_square)
        ++BracketCount;
    else if (BracketCount) {
        //AngleBrackets.clear(*this);
        --BracketCount;     // Don't let unbalanced ]'s drive the count negative.
    }

    PrevTokLocation = Tok.getLocation();
    Lex.Lex(Tok);
    return PrevTokLocation;
}

/**
 * ConsumeBrace - This consume method keeps the brace count up-to-date.
 * @return
 */
SourceLocation Parser::ConsumeBrace(unsigned short &BraceCount) {
    FLY_DEBUG_START("Parser", "ConsumeBrace");
    assert(isTokenBrace() && "wrong consume method");
    if (Tok.getKind() == tok::l_brace)
        ++BraceCount;
    else if (BraceCount) {
        //AngleBrackets.clear(*this);
        --BraceCount;     // Don't let unbalanced }'s drive the count negative.
    }

    PrevTokLocation = Tok.getLocation();
    Lex.Lex(Tok);
    return PrevTokLocation;
}

bool Parser::isBraceBalanced() const {
    FLY_DEBUG_START("Parser", "isBraceBalanced");
    return BraceCount == 0;
}

/**
 * ConsumeStringToken - Consume the current 'peek token', lexing a new one
 * and returning the token kind.  This method is specific to strings, as it
 * handles string literal concatenation, as per C99 5.1.1.2, translation
 * phase #6.
 * @return
 */
SourceLocation Parser::ConsumeStringToken() {
    FLY_DEBUG_START("Parser", "ConsumeStringToken");
    assert(isTokenStringLiteral() && "Should only consume string literals with this method");
    PrevTokLocation = Tok.getLocation();
    Lex.Lex(Tok);
    return PrevTokLocation;
}

/**
 * Get String between quotes from token
 * @return string
 */
llvm::StringRef Parser::getLiteralString() {
    FLY_DEBUG_START("Parser", "getLiteralString");
    StringRef Name(Tok.getLiteralData(), Tok.getLength());
    StringRef Str = "";
    if (Name.startswith("\"") && Name.endswith("\"")) {
        StringRef StrRefName = Name.substr(1, Name.size()-2);
        ConsumeStringToken();
        return StrRefName;
    }
    FLY_DEBUG_MESSAGE("Parser", "getLiteralString", "return " << Str);
    return Str;
}

/**
 * Print Diagnostic message by SourceLocation
 * @param Loc
 * @param DiagID
 * @return
 */
DiagnosticBuilder Parser::Diag(SourceLocation Loc, unsigned DiagID) {
    return Diags.Report(Loc, DiagID);
}

/**
 * Print Diagnostic message by Token
 * @param Tok
 * @param DiagID
 * @return
 */
DiagnosticBuilder Parser::Diag(const Token &Tok, unsigned DiagID) {
    return Diag(Tok.getLocation(), DiagID);
}

DiagnosticBuilder Parser::Diag(unsigned DiagID) {
    return Diag(Tok, DiagID);
}

void Parser::DiagInvalidId(SourceLocation Loc) {
    if (isKeyword(Tok.getKind())) {
        Diag(Loc, diag::err_invalid_id) << tok::getKeywordSpelling(Tok.getKind());
    } else {
        Diag(Loc, diag::err_invalid_id) << tok::getPunctuatorSpelling(Tok.getKind());
    }
}
