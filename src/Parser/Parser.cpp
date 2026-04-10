//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/Parser.cpp - Main Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/Parser.h"

#include "AST/ASTBlockStmt.h"
#include "AST/ASTBreakStmt.h"
#include "AST/ASTBuilder.h"
#include "AST/ASTBuilderIfStmt.h"
#include "AST/ASTBuilderLoopInStmt.h"
#include "AST/ASTBuilderLoopStmt.h"
#include "AST/ASTBuilderStmt.h"
#include "AST/ASTBuilderSwitchStmt.h"
#include "AST/ASTCall.h"
#include "AST/ASTClass.h"
#include "AST/ASTComment.h"
#include "AST/ASTContinueStmt.h"
#include "AST/ASTEnum.h"
#include "AST/ASTExpr.h"
#include "AST/ASTFunction.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTImport.h"
#include "AST/ASTModule.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTType.h"
#include "AST/ASTValue.h"
#include "AST/ASTVar.h"
#include "Basic/Debug.h"
#include "Frontend/InputFile.h"
#include "Parser/ParserClass.h"
#include "Parser/ParserEnum.h"
#include "Parser/ParserExpr.h"
#include "Parser/ParserFunction.h"

#include "llvm/Support/Regex.h"

#include <AST/ASTDeclStmt.h>
#include <AST/ASTExprStmt.h>
#include <AST/ASTFailStmt.h>
#include <AST/ASTLocalVar.h>
#include <AST/ASTModifier.h>
#include <AST/ASTReturnStmt.h>

using namespace fly;

/**
 * Parser Constructor
 * @param Input
 * @param SourceMgr
 * @param Diags
 */
Parser::Parser(InputFile *Input, SourceManager &SourceMgr, DiagnosticsEngine &Diags, ASTBuilder &Builder) :
    Input(Input), Diags(Diags), SourceMgr(SourceMgr), Builder(Builder),
    Lex(Input->getFileID(), Input->getBuffer(), SourceMgr), Tok(), Module(nullptr) {
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

    Module = ASTBuilder::CreateModule(Input);

    // Start with Parse (recursively))
    while (ContinueParse && Tok.isNot(tok::eof)) {

	    // Parse a NameSpace
    	if (Tok.is(tok::kw_namespace)) {
    		Module->setNameSpace(ParseNameSpace());
    	}

    	// Parse a Node
    	else {
    		ParseNode();
    	}
    }

    return Module;
}

ASTModule *Parser::ParseHeader() {
    FLY_DEBUG_START("Parser", "ParseHeader");
    Tok.startToken();
    Tok.setKind(tok::eof);

    // Prime the lexer look-ahead.
    ConsumeToken();

    return Module;
}

bool Parser::isSuccess() {
    return !Diags.hasErrorOccurred();
}

ASTNameSpace *Parser::ParseNameSpace() {
	FLY_DEBUG_START("Parser", "ParseNameSpace");
	const SourceLocation &Loc = Tok.getLocation();
	ConsumeToken(); // consume 'namespace'

	if (!Tok.isAnyIdentifier()) {
		Diag(Tok, diag::err_parser_identifier_expected);
		return nullptr;
	}

	// Parse the Names
	llvm::SmallVector<ASTName *, 4> Names = ParseNames();

	// Create NameSpace
	return ASTBuilder::CreateNameSpace(Module, Loc, Names);
}

ASTImport *Parser::ParseImport() {
	FLY_DEBUG_START("Parser", "ParseImport");

	const SourceLocation &Loc = Tok.getLocation();
	ConsumeToken(); // consume 'import'

	if (!Tok.isAnyIdentifier()) {
		Diag(Tok, diag::err_parser_identifier_expected);
		return nullptr;
	}

	// Parse the Names
	llvm::SmallVector<ASTName *, 4> Names = ParseNames();

	// Parse optional 'as' alias
	llvm::SmallVector<ASTName *, 4> Alias;
	if (Tok.is(tok::kw_as)) {

		// Consume 'as'
		SourceLocation AsLoc = Tok.getLocation();
		ConsumeToken();

		// Parse the Names
		Alias = ParseNames();
	}

	// Create Import
	return ASTBuilder::CreateImport(Module, Loc, Names, Alias);
}

llvm::SmallVector<ASTName *, 4> Parser::ParseNames() {
	FLY_DEBUG_START("Parser", "ParseNames");
	llvm::SmallVector<ASTName *, 4> Names;

	while (true) {
		if (!Tok.isAnyIdentifier()) {
			Diag(Tok, diag::err_parser_identifier_expected);
			break;
		}

		// Parse Name
		ASTName *Name = ASTBuilder::CreateName(Tok.getIdentifierInfo()->getName(), Tok.getLocation());
		Names.push_back(Name);
		ConsumeToken();

		// Check for period
		if (Tok.is(tok::period)) {
			ConsumeToken(); // consume '.'
		} else {
			break;
		}
	}

	return Names;
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
		Comment = ASTBuilder::CreateComment(Module, SourceLocation(), Lex.BlockComment);
	}

	return Comment;
}

void Parser::ParseNode() {
	FLY_DEBUG_START("Parser", "ParseNode");

	ASTNode *Node = nullptr;

	// Parse an Import
	if (Tok.is(tok::kw_import)) {
		ParseImport();
		return;
	}

	// Parse a Comment
	if (isTokenComment()) {
		ParseComment();
		return;
	}

	// Parse Top Modifiers: Public/Private and Constant
	SmallVector<ASTModifier *, 8> Modifiers = ParseModifiers();

	// Parse a Class
	if (Tok.isOneOf(tok::kw_struct, tok::kw_class, tok::kw_interface)) {
		ParseClass(Modifiers);
	}

	// Parse an Enum
	else if (Tok.is(tok::kw_enum)) {
		ParseEnum(Modifiers);
	}

    // Parse Identifier
	else {
		ParseFunction(Modifiers);
	}
}

SmallVector<ASTModifier *, 8> Parser::ParseModifiers() {
    FLY_DEBUG_START("ClassParser", "ParseModifiers");

    llvm::SmallVector<ASTModifier *, 8> Modifiers;

    while (Tok.isNot(tok::eof)) {
        if (Tok.is(tok::kw_private)) {
            Modifiers.push_back(ASTBuilder::CreateModifier(Tok.getLocation(), ASTModifierKind::MOD_PRIVATE));
        } else if (Tok.is(tok::kw_protected)) {
            Modifiers.push_back(ASTBuilder::CreateModifier(Tok.getLocation(), ASTModifierKind::MOD_PROTECTED));
        } else if (Tok.is(tok::kw_public)) {
            Modifiers.push_back(ASTBuilder::CreateModifier(Tok.getLocation(), ASTModifierKind::MOD_PUBLIC));
        } else if (Tok.is(tok::kw_const)) {
            Modifiers.push_back(ASTBuilder::CreateModifier(Tok.getLocation(), ASTModifierKind::MOD_CONSTANT));
        } else if (Tok.is(tok::kw_static)) {
            Modifiers.push_back(ASTBuilder::CreateModifier(Tok.getLocation(), ASTModifierKind::MOD_STATIC));
        } else if (Tok.is(tok::kw_abstract)) {
            Modifiers.push_back(ASTBuilder::CreateModifier(Tok.getLocation(), ASTModifierKind::MOD_ABSTRACT));
        } else if (Tok.is(tok::kw_final)) {
            Modifiers.push_back(ASTBuilder::CreateModifier(Tok.getLocation(), ASTModifierKind::MOD_FINAL));
        } else {
            break;
        }
    	ConsumeToken();
    }

    return Modifiers;
}

/**
 * ParseModule Class declaration
 * @param Visibility
 * @param Constant
 * @return
 */
ASTClass * Parser::ParseClass(SmallVector<ASTModifier *, 8> &Modifiers) {
	FLY_DEBUG_START("Parser", "ParseClass");

    ASTClass *Class = ParserClass::Parse(this, Modifiers);
    return Class;
}


/**
 * ParseModule Enum declaration
 * @param Visibility
 * @param Constant
 * @return
 */
ASTEnum *Parser::ParseEnum(SmallVector<ASTModifier *, 8>&Modifiers) {
	FLY_DEBUG_START("Parser", "ParseEnum");
    assert(Tok.is(tok::kw_enum) && "Token Enum expected");

    ASTEnum *Enum = ParserEnum::Parse(this, Modifiers);
    return Enum;
}

/**
 * ParseModule Function declaration
 * Functions are implicitly void - no return type declaration
 * @param Modifiers
 * @return
 */
ASTFunction *Parser::ParseFunction(SmallVector<ASTModifier *, 8> &Modifiers) {
	FLY_DEBUG_START("Parser", "ParseFunction");

	// Parse Function Name
	const SourceLocation &Loc = Tok.getLocation();
	StringRef Name = Tok.getIdentifierInfo()->getName();
	ConsumeToken();

	// Parse Params
	SmallVector<ASTParam *, 8> Params = ParserFunction::ParseParams(this);

	// Create Function (implicitly void - no return type)
	ASTFunction *Function = ASTBuilder::CreateFunction(Module, Loc, Name, Modifiers, Params, nullptr);
	ASTBlockStmt *Body = isBlockStart() ? ParserFunction::ParseBody(this, Function) : nullptr;
	return Function;
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
 * Parse a single statement like Variable declaration, assignment or Function invocation
 *
 * Examples:
 *   const int a
 *   a = ...
 *   a()
 *   int a = ...
 *
 * @param Parent The parent block statement
 */
void Parser::ParseStmt(ASTBlockStmt *Parent) {
	FLY_DEBUG_START("Parser", "ParseStmt");

	// ===== 1. CONTROL FLOW STATEMENTS =====
	// These keywords start specific statement types that need dedicated parsing

	if (Tok.is(tok::kw_if)) {
		ParseIfStmt(Parent);
		return;
	}

	if (Tok.is(tok::kw_switch)) {
		ParseSwitchStmt(Parent);
		return;
	}

	if (Tok.is(tok::kw_for)) {
		ParseForStmt(Parent);
		return;
	}

	if (Tok.is(tok::kw_while)) {
		ParseWhileStmt(Parent);
		return;
	}

	if (Tok.is(tok::kw_handle)) {
		ParseHandleStmt(Parent);
		return;
	}

	if (Tok.is(tok::kw_fail)) {
		ParseFailStmt(Parent);
		return;
	}

	// Parse return statement (no expression - just exits function without error)
	if (Tok.is(tok::kw_return)) {
		SourceLocation Loc = Tok.getLocation();
		ConsumeToken();
		ASTBuilder::CreateReturnStmt(Parent, Loc);
		return;
	}

	if (Tok.is(tok::kw_break)) {
		ASTBuilder::CreateBreakStmt(Parent, Tok.getLocation());
		ConsumeToken();
		return;
	}

	if (Tok.is(tok::kw_continue)) {
		ASTBuilder::CreateContinueStmt(Parent, Tok.getLocation());
		ConsumeToken();
		return;
	}

	// Check for error handling: "Type name handle { ... }"
	if (Tok.is(tok::kw_handle)) {
		ParseHandleStmt(Parent);
		return;
	}

	// ===== 2. VARIABLE DECLARATIONS AND ASSIGNMENTS =====
	// Parse modifiers that may apply to local variables
	SmallVector<ASTModifier *, 8> Modifiers = ParseModifiers();

	ASTIdentifier *Identifier = nullptr;
	std::optional<Token> LookAhead = Tok;

	// Try to parse a typed variable declaration: "Type name" or "Type name = expr"
	if (isVarDecl(LookAhead)) {
		// Parse: int x; | NS.Type x; | Vector<Int> y; | MyClass[] arr; | MyNS.MyClass obj = foo();
		ASTType *T = ParseType();

		// Extract variable name
		llvm::StringRef Name = Tok.getIdentifierInfo()->getName();
		const SourceLocation &Loc = Tok.getLocation();
		ConsumeToken();

		// Create local variable and associated identifier
		ASTLocalVar *LocalVar = ASTBuilder::CreateLocalVar(Loc, T, Name, Modifiers);
		Identifier = ASTBuilder::CreateIdentifier(LocalVar);

		// Check for initialization: "Type name = expr"
		if (isAssignOperator(Tok)) {
			ASTDeclStmt *DeclStmt = ASTBuilder::CreateDeclStmt(Parent, Tok.getLocation(), LocalVar);
			DeclStmt->setExpr(ParseExpr(Identifier));
			return;
		}

		// Declaration without initializer
		ASTBuilder::CreateDeclStmt(Parent, Identifier->getLocation(), LocalVar);
		return;
	}

	// Try to parse an identifier assignment: "name = expr" or "name += expr"
	LookAhead = Tok;
	if (isVarAssign(LookAhead)) {
		llvm::StringRef Name = Tok.getIdentifierInfo()->getName();
		const SourceLocation &Loc = Tok.getLocation();
		ConsumeToken();
		Identifier = ASTBuilder::CreateIdentifier(Loc, Name);

		// Must have an assignment operator
		if (isAssignOperator(Tok)) {
			ASTExprStmt *Stmt = ASTBuilder::CreateExprStmt(Parent, Identifier->getLocation());
			ASTExpr *Expr = ParseExpr(Identifier);
			Stmt->setExpr(Expr);
			return;
		}
	}

	// ===== 3. EXPRESSION STATEMENTS =====
	// Parse expressions like function calls, increment/decrement, new objects, etc.
	// Examples: a(); a++; ++a; new A();

	if (!Tok.is(tok::r_brace) && !Tok.is(tok::eof)) {
		ASTExpr *Expr = ParseExpr();
		if (Expr) {
			ASTExprStmt *Stmt = ASTBuilder::CreateExprStmt(Parent, Expr->getLocation());
			Stmt->setExpr(Expr);
		}
	}
}

bool Parser::isType(std::optional<Token> &NexTok) {
    FLY_DEBUG_START("Parser", "isType");
    if (!NexTok) return false;

    // If the current token is a builtin keyword, treat it as a type and
    // advance NexTok to the next token so callers always receive the token
    // immediately after the type.
    if (NexTok->isKeyword()) {
        auto K = NexTok->getKind();
        switch (K) {
            case tok::kw_bool:
            case tok::kw_byte:
            case tok::kw_ushort:
            case tok::kw_short:
            case tok::kw_uint:
            case tok::kw_int:
            case tok::kw_ulong:
            case tok::kw_long:
            case tok::kw_float:
            case tok::kw_double:
            case tok::kw_void:
            case tok::kw_string:
            case tok::kw_char:
            case tok::kw_error: {
                // Advance to the next token after the builtin type
                NexTok = Lexer::findNextToken(NexTok->getLocation(), SourceMgr);
                // Now fall through to array suffix parsing / rejection checks below
                break;
            }
            default:
                return false;
        }
    }

    // If not a builtin keyword, expect an identifier-based type (A or A.B.C)
    else {
        if (!NexTok || !NexTok->isAnyIdentifier())
            return false;

        // --- Parse A or A.B.C ---
        while (true) {
            // Expect identifier
            if (!NexTok->isAnyIdentifier())
                return false;

            // Advance to next token after identifier
            NexTok = Lexer::findNextToken(NexTok->getLocation(), SourceMgr);
            if (!NexTok)
                break;

            // If we see a dot, consume it and require another identifier
            if (NexTok->is(tok::period)) {
                NexTok = Lexer::findNextToken(NexTok->getLocation(), SourceMgr);
                if (!NexTok || !NexTok->isAnyIdentifier())
                    return false;
                continue;
            }

            break;
        }
    }

    // --- Parse [] suffixes ---
    while (NexTok && NexTok->is(tok::l_square)) {
        // Skip '['
        NexTok = Lexer::findNextToken(NexTok->getLocation(), SourceMgr);
        if (!NexTok)
            return false;

        // Skip optional size expression until matching ']'
        int ParenDepth = 0;
        while (NexTok && (ParenDepth > 0 || !NexTok->is(tok::r_square))) {
            if (NexTok->is(tok::l_paren)) {
                ++ParenDepth;
            } else if (NexTok->is(tok::r_paren) && ParenDepth > 0) {
                --ParenDepth;
            }
            NexTok = Lexer::findNextToken(NexTok->getLocation(), SourceMgr);
        }
        if (!NexTok || !NexTok->is(tok::r_square))
            return false;

        // Advance past ']'
        NexTok = Lexer::findNextToken(NexTok->getLocation(), SourceMgr);
    }

    // --- Reject things that are NOT allowed for types ---
    // Function call → NOT a type
    if (NexTok && NexTok->is(tok::l_paren))
        return false;

    // Postfix ops → NOT a type
    if (NexTok && NexTok->isOneOf(tok::plusplus, tok::minusminus))
        return false;

    // If the next token is or starts an expression piece, it's not a pure type
    if (NexTok && isAnyOperator(NexTok.value()))
        return false;

    return true;
}

bool Parser::isVarDecl(std::optional<Token> &NexTok) {
	// Var Decl must start with a Type
	if (!isType(NexTok)) {
		return false;
	}

	// After isType returns, NexTok points to the token after the type (either
	// advanced by isType for builtin types or left advanced for named types).
	// Check that the next token is an identifier (the variable name).
	if (!NexTok || !NexTok->isAnyIdentifier())
		return false;

	return true;
}

bool Parser::isVarAssign(std::optional<Token> &NexTok) const {
	FLY_DEBUG_START("Parser", "isVarAssign");

	if (NexTok && NexTok->isAnyIdentifier()) {
		NexTok = Lexer::findNextToken(NexTok->getLocation(), SourceMgr);
		if (!NexTok)
			return false;

		return isAssignOperator(*NexTok);
	}

	return false;
}

bool Parser::isVar(std::optional<Token> &NexTok) {
    // Var must start with an Identifier
    if (!NexTok || !NexTok->isAnyIdentifier())
        return false;

    NexTok = Lexer::findNextToken(NexTok->getLocation(), SourceMgr);

    // --- Parse A or A.B.C ---
    while (true) {
        // If we see a dot, consume it and require another identifier
        if (NexTok && NexTok->is(tok::period)) {
            // Consume '.'
            NexTok = Lexer::findNextToken(NexTok->getLocation(), SourceMgr);
            if (!NexTok || !NexTok->isAnyIdentifier())
                return false;

            // Advance
            NexTok = Lexer::findNextToken(NexTok->getLocation(), SourceMgr);
            continue;
        }

        break;
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
    FLY_DEBUG_START_MSG("Parser", "ParseStartParen", "HasParen=" << HasParen);

    if (HasParen) {
        if (Tok.is(tok::r_paren)) {
            ConsumeParen();
        } else {
            Diag(diag::err_parser_right_paren);
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
	SourceLocation IfLoc = Tok.getLocation();
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
    ASTBuilderIfStmt *IfBuilder = ASTBuilderIfStmt::Create(Parent);
    ASTBlockStmt *IfBlock = ASTBuilder::CreateBlockStmt(Tok.getLocation());
    IfBuilder->If(IfLoc, IfCondition, IfBlock);

    // Parse statement between braces for If
    ParseBlockOrStmt(IfBlock);

    // Add Elsif
    while (Tok.is(tok::kw_elsif)) {
        const SourceLocation &ElsifLoc = Tok.getLocation();
    	ConsumeToken();

        // Parse (
        bool hasElsifParen = ParseStartParen();
        // Parse the group of expressions into parenthesis
        ASTExpr *ElsifCondition = ParseExpr(); // Parse Expr
        if (hasElsifParen) {
            ParseEndParen(hasElsifParen);
        }

        ASTBlockStmt *ElsifBlock = ASTBuilder::CreateBlockStmt(Tok.getLocation());
        ParseBlockOrStmt(ElsifBlock);

        IfBuilder->ElseIf(ElsifLoc, ElsifCondition, ElsifBlock);
    }

    // Add Else
    if (Tok.is(tok::kw_else)) {
        const SourceLocation &ElseLoc = ConsumeToken();
        ASTBlockStmt *ElseBlock = ASTBuilder::CreateBlockStmt(ElseLoc);
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

	// Consume switch keyword
	const SourceLocation &SwitchLoc = Tok.getLocation();
	ConsumeToken();

    // Parse (
    bool hasParen = ParseStartParen();

    // Parse the group of expressions
    ASTExpr *Expr = ParseExpr();

	// Parse )
	if (hasParen) {
		ParseEndParen(hasParen);
	}

	// Create Switch
	ASTBuilderSwitchStmt *SwitchBuilder = ASTBuilderSwitchStmt::Create(Parent, SwitchLoc, Expr);

    // Init Switch Statement and start parse from brace
    if (isBlockStart()) {
		ConsumeBrace(BracketCount);

    	while (true) {
    		if (Tok.is(tok::kw_case)) {

    			// Parse Case
    			SourceLocation CaseLoc = Tok.getLocation();
    			ConsumeToken();

    			// Parse Case Expr
    			ASTExpr *Expr = ParseExpr();

				// Parse Switch
				ASTBlockStmt *CaseBlock = ASTBuilder::CreateBlockStmt(Tok.getLocation());
				if (Tok.is(tok::colon)) { // Parse a Block of Stmt
					ConsumeToken();
					SwitchBuilder->addCase(CaseLoc, Expr, CaseBlock);
					if (Tok.isOneOf(tok::kw_case, tok::kw_default)) {
						continue;
					}
					// Only parse statement if we're not at the end of the switch block
					if (!Tok.is(tok::r_brace)) {
						ParseBlockOrStmt(CaseBlock);
					}
				}
    		} else if (Tok.is(tok::kw_default)) {
    			SourceLocation DefaultLoc = Tok.getLocation();
    			ConsumeToken();

    			if (SwitchBuilder->hasDefault()) {
    				Diag(Tok.getLocation(), diag::err_parser_syntax_default_error);
    				return;
    			}

				// Parse Default
				ASTBlockStmt *DefaultBlock = ASTBuilder::CreateBlockStmt(DefaultLoc);
				if (Tok.is(tok::colon)) { // Parse a Block of Stmt
					ConsumeToken();
					SwitchBuilder->setDefault(SwitchLoc, DefaultBlock);
					if (Tok.is(tok::kw_case)) {
						continue;
					}
					// Only parse statement if we're not at the end of the switch block
					if (!Tok.is(tok::r_brace)) {
						ParseBlockOrStmt(DefaultBlock);
					}
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

    Diag(diag::err_parser_syntax_error);
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

    const SourceLocation &WhileLoc = Tok.getLocation();
	ConsumeToken();

    // Consume Left Parenthesis ( if exists
    bool hasParen = ParseStartParen();

	// Create AST While Block
	ASTExpr *Condition = ParseExpr();

	// Parse )
	if (hasParen) {
		ParseEndParen(hasParen);
	}

    ASTBlockStmt *BlockStmt = ASTBuilder::CreateBlockStmt(Tok.getLocation());
    ASTBuilderLoopStmt *LoopBuilder = ASTBuilderLoopStmt::Create(Parent, WhileLoc);
    LoopBuilder->setCycle(Condition, BlockStmt);

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

    const SourceLocation &ForLoc = Tok.getLocation();
	ConsumeToken();

    // Consume Left Parenthesis ( if exists
    bool hasParen = ParseStartParen();

    // Check if this is a for-in loop: for <identifier> in <expr>
    // We need to look ahead to see if we have: identifier 'in'
    if (Tok.isAnyIdentifier()) {
        // Peek ahead to see if next token is 'in'
        std::optional<Token> NextTok = Lexer::findNextToken(Tok.getLocation(), SourceMgr);
        bool isForIn = false;

        if (NextTok) {
            // Check if it's the keyword 'in' or identifier "in"
            if (NextTok->is(tok::kw_in)) {
                isForIn = true;
            }
        }

        if (isForIn) {
            // Parse for-in loop: for item in list { }

            // Parse item identifier (the loop variable)
            llvm::StringRef ItemName = Tok.getIdentifierInfo()->getName();
            const SourceLocation &ItemLoc = Tok.getLocation();
        	ConsumeToken();
            ASTExpr *Item = ASTBuilder::CreateIdentifier(ItemLoc, ItemName);

            // Consume 'in'
        	SourceLocation InLoc = Tok.getLocation();
            ConsumeToken();

            // Parse list expression
            ASTExpr *List = ParseExpr();

            if (hasParen) {
                ParseEndParen(hasParen);
            }

            // Prepare loop body
            ASTBlockStmt *LoopBlock = ASTBuilder::CreateBlockStmt(Tok.getLocation());

            // Create LoopIn AST node
            ASTBuilderLoopInStmt *Builder= ASTBuilderLoopInStmt::Create(Parent, ForLoc, Item, List, LoopBlock);

            // Parse the loop body
            ParseBlockOrStmt(LoopBlock);
            return;
        }
    }

    // Traditional for loop: for init; condition; post { }

    // Create For Statement
	ASTBuilderLoopStmt *LoopBuilder = ASTBuilderLoopStmt::Create(Parent, ForLoc);
    ASTBlockStmt *InitBlock = ASTBuilder::CreateBlockStmt(SourceLocation());
    ASTExpr *Condition = nullptr;

    // for int a = 1, b = 2; i < 0; i++

	ParseStmt(InitBlock);
	while (Tok.is(tok::comma)) {
		ConsumeToken();
		ParseStmt(InitBlock);
	}

    // This is an Expression, it could be a Condition
	ASTBlockStmt *PostBlock = ASTBuilder::CreateBlockStmt(SourceLocation());
    if (Tok.is(tok::semi)) {
        ConsumeToken();

        Condition = ParseExpr();

        if (Tok.is(tok::semi)) {

        	ConsumeToken();
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
    ASTBlockStmt *LoopBlock = ASTBuilder::CreateBlockStmt(Tok.getLocation());
	LoopBuilder->setInit(InitBlock);
	LoopBuilder->setPost(PostBlock);
    LoopBuilder->setCycle(Condition, LoopBlock);

    // Parse statement between braces
    ParseBlockOrStmt(LoopBlock);
}

void Parser::ParseHandleStmt(ASTBlockStmt *Parent) {
	FLY_DEBUG_START("Parser", "ParseHandleStmt");
    assert(Tok.is(tok::kw_handle) && "Token is handle keyword");

    // Consume handle keyword
    const SourceLocation &HandleLoc = Tok.getLocation();
	ConsumeToken();

    // Parse statement between braces
    ASTBlockStmt *HandleBlock = ASTBuilder::CreateBlockStmt(HandleLoc);
    ParseBlockOrStmt(HandleBlock);
    ASTBuilder::CreateHandleStmt(Parent, HandleLoc, HandleBlock);
}

void Parser::ParseFailStmt(ASTBlockStmt *Parent) {
    FLY_DEBUG_START("Parser", "ParseFailStmt");
    assert(Tok.is(tok::kw_fail) && "Token is handle keyword");

    const SourceLocation &FailLoc = Tok.getLocation();
	ConsumeToken();
    ASTFailStmt *Stmt = ASTBuilder::CreateFailStmt(Parent, FailLoc);

    // Parse optional expression (fail can be used without an expression)
    if (!Tok.isOneOf(tok::r_brace, tok::eof, tok::kw_case, tok::kw_default,
                     tok::kw_break, tok::kw_continue, tok::kw_return,
                     tok::kw_if, tok::kw_switch, tok::kw_while, tok::kw_for)) {
        ASTExpr *FirstExpr = ParseExpr();
        Stmt->setFirstExpr(FirstExpr);

    	if (Tok.is(tok::comma)) {
    		ConsumeToken(); // consume ','
    		ASTExpr *SecondExpr = ParseExpr();
    		Stmt->setSecondExpr(SecondExpr);

    		if (Tok.is(tok::comma)) {
    			ConsumeToken(); // consume ','
    			ASTExpr *ThirdExpr = ParseExpr();
    			Stmt->setThirdExpr(ThirdExpr);

    			if (Tok.is(tok::comma)) {
    				Diag(Tok.getLocation(), diag::err_parser_fail_too_many_expr);
    			}
    		}
    	}
    }
}

/**
 * ParseModule a data Type
 * @return true on Success or false on Error
 */
ASTType *Parser::ParseType() {
    FLY_DEBUG_START("Parser", "ParseType");

	ASTType *T = nullptr;

	// Check Keyword for built-in Types
	if (Tok.isKeyword()) {
		switch (Tok.getKind()) {
			case tok::kw_bool:
				T = ASTBuilder::CreateBoolType(Tok.getLocation());
				break;
			case tok::kw_byte:
				T = ASTBuilder::CreateByteType(Tok.getLocation());
				break;
			case tok::kw_ushort:
				T = ASTBuilder::CreateUShortType(Tok.getLocation());
				break;
			case tok::kw_short:
				T = ASTBuilder::CreateShortType(Tok.getLocation());
				break;
			case tok::kw_uint:
				T = ASTBuilder::CreateUIntType(Tok.getLocation());
				break;
			case tok::kw_int:
				T = ASTBuilder::CreateIntType(Tok.getLocation());
				break;
			case tok::kw_ulong:
				T = ASTBuilder::CreateULongType(Tok.getLocation());
				break;
			case tok::kw_long:
				T = ASTBuilder::CreateLongType(Tok.getLocation());
				break;
			case tok::kw_float:
				T = ASTBuilder::CreateFloatType(Tok.getLocation());
				break;
			case tok::kw_double:
				T = ASTBuilder::CreateDoubleType(Tok.getLocation());
				break;
			case tok::kw_void:
				T = ASTBuilder::CreateVoidType(Tok.getLocation());
				break;
			case tok::kw_string:
				T = ASTBuilder::CreateStringType(Tok.getLocation());
				break;
			case tok::kw_error:
				T = ASTBuilder::CreateErrorType(Tok.getLocation());
				break;
		}
		ConsumeToken();
	}

	// Parse Class or Enum Type
	else if (Tok.isAnyIdentifier()) {
		llvm::SmallVector<ASTName *, 4> Names = ParseNames();
		T = ASTBuilder::CreateType(Tok.getLocation(), Names);
	}

	// Parse Array Type
	while (Tok.is(tok::l_square)) {
		SourceLocation Loc = ConsumeBracket();

		// Parse Size Expression
		ASTExpr *Size = nullptr;
		if (Tok.isNot(tok::r_square)) {
			Size = ParseExpr();
		}

		T = ASTBuilder::CreateArrayType(Loc, T, Size);

		if (Tok.is(tok::r_square)) {
			ConsumeBracket();
		} else {
			Diag(Tok, diag::err_parser_unclosed_bracket);
			return T;
		}
	}

    return T;
}

ASTExpr *Parser::ParseExpr(ASTExpr *Left) {
    FLY_DEBUG_START("Parser", "ParseExpr");
    ParserExpr PE(this);
    return PE.Parse(Left);
}

ASTExpr *Parser::ParseIdentifier() {
	FLY_DEBUG_START("Parser", "ParseIdentifier");
	ParserExpr PE(this);
	return PE.ParseIdentifierOrCall();
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
    return Tok.isOneOf(tok::numeric_constant, tok::kw_true, tok::kw_false, tok::kw_null, tok::kw_unset, tok::l_brace,
                       tok::char_constant, tok::string_literal);
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

bool Parser::isAnyOperator(const Token &Tok) const {
	FLY_DEBUG_START("Parser", "isAnyOperator");
	return Tok.isOneOf(tok::plus, tok::minus, tok::star, tok::slash, tok::percent,
                   tok::amp, tok::pipe, tok::caret,
                   tok::exclaim, tok::less, tok::greater,
                   tok::equalequal, tok::exclaimequal,
                   tok::lessequal, tok::greaterequal,
                   tok::lessless, tok::greatergreater,
                   tok::plusplus, tok::minusminus);
}

bool Parser::isAssignOperator(Token &Tok) const {
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
    if (Name.starts_with("\"") && Name.ends_with("\"")) {
        StringRef StrRefName = Name.substr(1, Name.size()-2);
        ConsumeStringToken();
        return StrRefName;
    }
    FLY_DEBUG_START_MSG("Parser", "getLiteralString", "return " << Str);
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
        Diag(Loc, diag::err_parser_invalid_id) << tok::getKeywordSpelling(Tok.getKind());
    } else {
        Diag(Loc, diag::err_parser_invalid_id) << tok::getPunctuatorSpelling(Tok.getKind());
    }
}
