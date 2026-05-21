//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/ExprParser.cpp - Expression Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/ParserExpr.h"

#include "AST/ASTBinary.h"
#include "AST/ASTBuilder.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTTernary.h"
#include "AST/ASTUnary.h"
#include "Basic/Debug.h"

#include <AST/ASTCall.h>
#include <AST/ASTMember.h>
#include <AST/ASTValue.h>

using namespace fly;

Precedence getPrecedence(Token Tok) {
	FLY_DEBUG_SCOPE("ParserExpr", "getPrecedence");
    switch (Tok.getKind()) {
        case fly::tok::question:
            return Precedence::TERNARY;
        case tok::equal:
        case tok::plusequal:
        case tok::minusequal:
        case tok::starequal:
        case tok::slashequal:
        case tok::percentequal:
        case tok::ampequal:
        case tok::pipeequal:
        case tok::lesslessequal:
        case tok::greatergreaterequal:
        case tok::caretequal:
            return Precedence::ASSIGNMENT;
        case tok::pipepipe:
        case tok::pipe:
        case tok::ampamp:
        case tok::amp:
        case tok::caret:
        case tok::lessless:
        case tok::greatergreater:
            return Precedence::LOGICAL;
        case tok::equalequal:
        case tok::exclaimequal:
        case tok::less:
        case tok::greater:
        case tok::lessequal:
        case tok::greaterequal:
            return Precedence::RELATIONAL;
        case tok::plus:
        case tok::minus:
            return Precedence::ADDITIVE;
        case tok::star:
        case tok::percent:
        case tok::slash:
            return Precedence::MULTIPLICATIVE;
        default:
            return Precedence::LOWEST;
    }
}

bool isRightAssociative(Token Tok) {
	FLY_DEBUG_SCOPE("ParserExpr", "isRightAssociative");
    // Only assignment operators are right-associative
    return Tok.isOneOf(tok::equal, tok::plusequal, tok::minusequal, tok::starequal, tok::slashequal, tok::percentequal,
                       tok::ampequal, tok::pipeequal, tok::caretequal, tok::lesslessequal, tok::greatergreaterequal);
}

ASTUnaryKind toUnaryOpExprKind(Token Tok, bool isPost) {
	FLY_DEBUG_SCOPE("ParserExpr", "toUnaryOpExprKind");
    if (isPost) {
        switch (Tok.getKind()) {
            case tok::plusplus:
                return ASTUnaryKind::OP_UNARY_POST_INCR;
            case tok::minusminus:
                return ASTUnaryKind::OP_UNARY_POST_DECR;
        }
    } else {
        switch (Tok.getKind()) {
            case tok::plusplus:
                return ASTUnaryKind::OP_UNARY_PRE_INCR;
            case tok::minusminus:
                return ASTUnaryKind::OP_UNARY_PRE_DECR;
            case tok::exclaim:
                return ASTUnaryKind::OP_UNARY_NOT_LOG;
        }
    }
    assert(false && "Invalid Unary Token details");
}

ASTBinaryKind toBinaryOpExprKind(Token Tok) {
	FLY_DEBUG_SCOPE("ParserExpr", "toBinaryOpExprKind");
    switch (Tok.getKind()) {
        case tok::amp:
            return ASTBinaryKind::OP_BINARY_ARITH_AND;
        case tok::ampamp:
            return ASTBinaryKind::OP_BINARY_LOGIC_AND;
        case tok::ampequal:
            return ASTBinaryKind::OP_BINARY_ASSIGN_AND;
        case tok::star:
            return ASTBinaryKind::OP_BINARY_ARITH_MUL;
        case tok::starequal:
            return ASTBinaryKind::OP_BINARY_ASSIGN_MUL;
        case tok::plus:
            return ASTBinaryKind::OP_BINARY_ARITH_ADD;
        case tok::plusequal:
            return ASTBinaryKind::OP_BINARY_ASSIGN_ADD;
        case tok::minus:
            return ASTBinaryKind::OP_BINARY_ARITH_SUB;
        case tok::minusequal:
            return ASTBinaryKind::OP_BINARY_ASSIGN_SUB;
        case tok::exclaimequal:
            return ASTBinaryKind::OP_BINARY_COMPARE_NE;
        case tok::slash:
            return ASTBinaryKind::OP_BINARY_ARITH_DIV;
        case tok::slashequal:
            return ASTBinaryKind::OP_BINARY_ASSIGN_DIV;
        case tok::percent:
            return ASTBinaryKind::OP_BINARY_ARITH_MOD;
        case tok::percentequal:
            return ASTBinaryKind::OP_BINARY_ASSIGN_MOD;
        case tok::less:
            return ASTBinaryKind::OP_BINARY_COMPARE_LT;
        case tok::lessless:
            return ASTBinaryKind::OP_BINARY_ARITH_SHIFT_L;
        case tok::lessequal:
            return ASTBinaryKind::OP_BINARY_COMPARE_LTE;
        case tok::lesslessequal:
            return ASTBinaryKind::OP_BINARY_ASSIGN_SHIFT_L;
        case tok::greater:
            return ASTBinaryKind::OP_BINARY_COMPARE_GT;
        case tok::greatergreater:
            return ASTBinaryKind::OP_BINARY_ARITH_SHIFT_R;
        case tok::greaterequal:
            return ASTBinaryKind::OP_BINARY_COMPARE_GTE;
        case tok::greatergreaterequal:
            return ASTBinaryKind::OP_BINARY_ASSIGN_SHIFT_R;
        case tok::caret:
            return ASTBinaryKind::OP_BINARY_ARITH_XOR;
        case tok::caretequal:
            return ASTBinaryKind::OP_BINARY_ASSIGN_XOR;
        case tok::pipe:
            return ASTBinaryKind::OP_BINARY_ARITH_OR;
        case tok::pipepipe:
            return ASTBinaryKind::OP_BINARY_LOGIC_OR;
        case tok::pipeequal:
            return ASTBinaryKind::OP_BINARY_ASSIGN_OR;
        case tok::equal:
            return ASTBinaryKind::OP_BINARY_ASSIGN;
        case tok::equalequal:
            return ASTBinaryKind::OP_BINARY_COMPARE_EQ;
    }
    assert(false && "Invalid Binary Token details");
}

ParserExpr::ParserExpr(Parser *P, ASTExpr *Left) : P(P), Left(Left) {
	FLY_DEBUG_SCOPE("ParserExpr", "ParserExpr");
}

ASTExpr *ParserExpr::Parse(ASTExpr *Left) {
	FLY_DEBUG_SCOPE("ParserExpr", "Parse");
	// Parse the primary expression (handles parentheses, literals, identifiers, and now unary operators)
	if (Left == nullptr)
		Left = ParsePrimary();

	// Expr contains a binary or ternary operator
	if (isBinaryOperator() || isTernaryOperator()) {

		// Start with the lowest precedence
		Precedence precedence = Precedence::LOWEST;

		while (true) {
			Token OpTok = P->Tok;
			Precedence nextPrecedence = getPrecedence(OpTok);

			// If the next operator has lower precedence, stop parsing
			if (nextPrecedence == Precedence::LOWEST || nextPrecedence < precedence) {
				break;
			}

			// Handle binary expression or ternary expression
			if (isTernaryOperator()) {
				Left = ParseTernaryExpr(Left);  // Handle ternary operators
			} else {
				Left = ParseBinaryExpr(Left, OpTok, nextPrecedence);  // Handle binary expressions
			}
		}
	}

	return Left;
}


ASTExpr * ParserExpr::ParseIdentifierOrCall(ASTExpr *Parent) {
	llvm::StringRef Name = P->Tok.getIdentifierInfo()->getName();
	const SourceLocation &Loc =P->Tok.getLocation() ;
	P->ConsumeToken();

	ASTExpr *Expr;
	if (P->Tok.is(tok::l_paren)) {
		Expr = ParseCall(Loc, Name, ASTCallKind::CALL_DIRECT, Parent);
	} else if (Parent) {
		Expr = ASTBuilder::CreateMember(Loc, Name, Parent);
	} else {
		Expr = ASTBuilder::CreateIdentifier(Loc, Name);
	}

	// Handle member access chaining
	if (P->Tok.is(tok::period)) {
		P->ConsumeToken();

		// Allow keywords (e.g. "string") as namespace/member components after '.'
		if (!P->Tok.isAnyIdentifier() && !P->Tok.getIdentifierInfo()) {
			P->Diag(P->Tok.getLocation(), diag::err_parser_identifier_expected);
		} else {
			return ParseIdentifierOrCall(Expr);
		}
	}

	return Expr;
}


/**
 * ParseModule a Value Expression
 * @return the ASTValueExpr
 */
ASTValue *ParserExpr::ParseValue() {
    FLY_DEBUG_SCOPE("ParserExpr", "ParseValue");

    if (P->Tok.isLiteral()) {
        StringRef Literal = StringRef(P->Tok.getLiteralData(), P->Tok.getLength());
        if (Literal == "''") {
            const SourceLocation &Loc = P->ConsumeToken();
            return ASTBuilder::CreateStringValue(Loc, "");
        }
    }

    if (P->Tok.is(tok::kw_null)) {
        const SourceLocation &Loc = P->ConsumeToken();
        return ASTBuilder::CreateNullValue(Loc);
    }

    if (P->Tok.is(tok::kw_unset)) {
        const SourceLocation &Loc = P->ConsumeToken();
        return ASTBuilder::CreateUnsetValue(Loc);
    }

    // Parse Numeric Constants
    if (P->Tok.is(tok::numeric_constant)) {
        llvm::StringRef Val = llvm::StringRef(P->Tok.getLiteralData(), P->Tok.getLength());
        const SourceLocation &Loc = P->ConsumeToken();
        return ASTBuilder::CreateNumberValue(Loc, Val);
    }

    if (P->Tok.isCharLiteral()) {
        assert(P->Tok.isLiteral() && "char literal must report literal data");
        llvm::StringRef Val = llvm::StringRef(P->Tok.getLiteralData(), P->Tok.getLength());
        return ASTBuilder::CreateStringValue(P->ConsumeToken(), Val);
    }

    if (P->Tok.isStringLiteral()) {
        llvm::StringRef Val = llvm::StringRef(P->Tok.getLiteralData(), P->Tok.getLength());
        return ASTBuilder::CreateStringValue(P->ConsumeStringToken(), Val);
    }

	// Parse true or false boolean values
	if (P->Tok.is(tok::kw_true)) {
		return ASTBuilder::CreateBoolValue(P->ConsumeToken(), true);
	}
	if (P->Tok.is(tok::kw_false)) {
		return ASTBuilder::CreateBoolValue(P->ConsumeToken(), false);
	}

	// Parse Array or Struct
	if (P->Tok.is(tok::l_brace)) {
		return ParseValues();
	}

	P->Diag(diag::err_parser_invalid_value) << P->Tok.getName();
	return nullptr;
}

ASTExpr *ParserExpr::ParsePrimary() {
	FLY_DEBUG_SCOPE("ParserExpr", "ParsePrimary");
    Token &Tok = P->Tok;

    // Parse Value
    if (P->isValue()) { // Ex. 1
        return ParseValue();
    }

	// Parse Identifier or Call
	if (P->Tok.isAnyIdentifier()) { // Ex. a or a++ or func()

		ASTExpr *Primary = ParseIdentifierOrCall();

		// parse function call, variable post increment/decrement or simple var
		if (isUnaryPostOperator()) { // Ex. a++ or a--
			ASTUnaryKind OpKind = toUnaryOpExprKind(Tok, true);
			SourceLocation OpLoc = Tok.getLocation();
			P->ConsumeToken();
			return ASTBuilder::CreateUnary(OpLoc, OpKind, Primary);
		}

		return Primary;
	}

	if (isUnaryPreOperator(P->Tok)) { // Ex. ++a or --a or !a
		ASTUnaryKind OpKind = toUnaryOpExprKind(Tok, false);
		const SourceLocation &OpLoc = P->ConsumeToken();
		ASTExpr* Primary = ParsePrimary();  // Parse the operand (recursively)
		return ASTBuilder::CreateUnary(OpLoc, OpKind, Primary);
	}

	// Parse New Expression
	if (isNewOperator(P->Tok)) {
		return ParseNewExpr();
	}

	// Parse Parentheses
	if (P->Tok.is(tok::l_paren)) {
		P->ConsumeParen();
		ParserExpr PE(P);
		ASTExpr *Primary = PE.Parse();
		if (P->Tok.is(tok::r_paren)) {
			P->ConsumeParen();
		} else {
			P->Diag(P->Tok.getLocation(), diag::err_parser_expr_close_paren);
		}
		return Primary;
	}

	P->Diag(P->Tok.getLocation(), diag::err_parser_expr_expected_primary);
    // Consume token to avoid parser stalling in callers that expect progress
    P->ConsumeToken();

    return nullptr;
}

ASTBinary *ParserExpr::ParseBinaryExpr(ASTExpr *LeftExpr, Token OpToken, Precedence precedence) {
	FLY_DEBUG_SCOPE("ParserExpr", "ParseBinaryExpr");

    // Consume the binary operator
    P->ConsumeToken();

    // Parse the right-hand side of the binary expression
    ASTExpr* RightExpr = ParsePrimary();  // Parse the RHS (which may include parentheses)

    // Keep climbing the RHS while the next operator binds tighter than the current one.
    // A single `if` only absorbed one level; the `while` handles chains like
    // `x = a * a + b * b` where both `*` and `+` have higher precedence than `=`.
    while (true) {
        Token NextTok = P->Tok;
        Precedence nextPrecedence = getPrecedence(NextTok);
        if (nextPrecedence == Precedence::LOWEST) break;
        // Ternary '?' must be handled by the outer ParseExpr loop, not consumed here.
        if (nextPrecedence == Precedence::TERNARY) break;
        if (!(nextPrecedence > precedence ||
              (nextPrecedence == precedence && isRightAssociative(OpToken)))) break;
        RightExpr = ParseBinaryExpr(RightExpr, NextTok, nextPrecedence);
    }

    // Combine the left and right into a binary operation node

    return ASTBuilder::CreateBinary(OpToken.getLocation(), toBinaryOpExprKind(OpToken), LeftExpr, RightExpr);
}

ASTTernary *ParserExpr::ParseTernaryExpr(ASTExpr *ConditionExpr) {
	FLY_DEBUG_SCOPE("ParserExpr", "ParseTernaryExpr");
    const SourceLocation &TrueOpLoc = P->ConsumeToken();  // Consume '?'

	ParserExpr PET(P);
    ASTExpr* TrueExpr = PET.Parse();  // Parse the true expression

    if (P->Tok.isNot(tok::colon)) {
        throw P->Diag(P->Tok.getLocation(), diag::err_parser_ternary_expr);
    }

    const SourceLocation &FalseOpLoc = P->ConsumeToken();  // Consume ':'

	ParserExpr PEF(P);
    ASTExpr* FalseExpr = PEF.Parse();  // Parse the false expression

    return ASTBuilder::CreateTernary(ConditionExpr, TrueOpLoc, TrueExpr, FalseOpLoc, FalseExpr);
}

/**
 * Check if Token is one of the Unary Pre Operators
 * @return true on Success or false on Error
 */
bool ParserExpr::isNewOperator(Token &Tok) {
    FLY_DEBUG_SCOPE("ParserExpr", "isNewOperator");
    return Tok.is(tok::kw_new);
}

/**
 * Check if Token is one of the Unary Pre Operators
 * @return true on Success or false on Error
 */
bool ParserExpr::isUnaryPreOperator(Token &Tok) {
    FLY_DEBUG_SCOPE("ParserExpr", "isUnaryPreOperator");
    return Tok.isOneOf(tok::plusplus, tok::minusminus, tok::exclaim);
}

/**
 * Check if Token is one of the Unary Post Operators
 * @return true on Success or false on Error
 */
bool ParserExpr::isUnaryPostOperator() {
    FLY_DEBUG_SCOPE("ParserExpr", "isUnaryPostOperator");
    return P->Tok.isOneOf(tok::plusplus, tok::minusminus);
}

/**
 * Check if Token is one of Binary Operators
 * @return true on Success or false on Error
 */
bool ParserExpr::isBinaryOperator() {
    FLY_DEBUG_SCOPE("ParserExpr", "isBinaryOperator");
    return P->Tok.isOneOf(

            // Arithmetic Operators
            tok::plus, // + add
            tok::minus, // - subtract
            tok::star, // * multiply
            tok::slash, // / divide
            tok::percent, // % percentage

            // Logic Operators
            tok::ampamp, // && logic and
            tok::pipepipe, // || logic or
            tok::less, // <
            tok::greater, // >
            tok::lessequal, // <= less than
            tok::greaterequal, // >= greater than
            tok::equalequal, // == equal compare
            tok::exclaimequal, // != different compare

            // Bit operators
            tok::amp, // & and
            tok::pipe, // | or
            tok::caret, // ^ xor
            tok::lessless, // << shift left
            tok::greatergreater, // >> shift right

            // Assignment
            tok::equal,
            tok::plusequal,
            tok::minusequal,
            tok::starequal,
            tok::slashequal,
            tok::percentequal,
            tok::ampequal,
            tok::pipeequal,
            tok::lesslessequal,
            tok::greatergreaterequal,
            tok::caretequal
    );
}

/**
 * Check if Token is one of Binary Operators
 * @return true on Success or false on Error
 */
bool ParserExpr::isTernaryOperator() {
    FLY_DEBUG_SCOPE("ParserExpr", "isTernaryOperator");
    return P->Tok.is(tok::question);
}

ASTExpr *ParserExpr::ParseNewExpr() {
    FLY_DEBUG_SCOPE("ParserExpr", "ParseNewExpr");
    const SourceLocation &NewOpLoc = P->ConsumeToken();  // Consume 'new'

    if (P->Tok.isAnyIdentifier()) {
    	llvm::StringRef Name = P->Tok.getIdentifierInfo()->getName();
    	const SourceLocation &Loc = P->ConsumeToken();
    	return ParseCall(Loc, Name, ASTCallKind::CALL_NEW);
    }

    // Error:
    P->Diag(P->Tok.getLocation(), diag::err_parser_new_instance);
    return nullptr;
}

/**
 * ParseModule a Function Call
 * @param Block
 * @param Id
 * @param Loc
 * @return true on Success or false on Error
 */
ASTCall *ParserExpr::ParseCall(const SourceLocation &Loc, llvm::StringRef Name, ASTCallKind CallKind, ASTExpr *Parent) {
	FLY_DEBUG_SCOPE("Parser", "ParseCall");
	assert(P->Tok.is(tok::l_paren) && "Call start with parenthesis");

	// Parse Call args
	P->ConsumeParen(); // consume l_paren

	// Parse Args in a Function Call
	llvm::SmallVector<ASTExpr *, 8> Args;
	while (true) {
		// Check for closing parenthesis (end of parameter List)
		if (P->Tok.is(tok::r_paren)) {
			P->ConsumeParen();
			break;
		}

		// Parse a parameter
		ParserExpr PE(P);
		ASTExpr *Arg = PE.Parse();
		if (Arg == nullptr) {
			// Handle error: Invalid parameter syntax
			P->Diag(P->Tok.getLocation(), diag::err_parser_invalid_param);
			break;
		}

		// Add the parsed parameter to the List
		Args.push_back(Arg);

		// Check for a comma (',') to separate parameters
		if (P->Tok.is(tok::comma)) {
			P->ConsumeToken(); // Consume the comma and continue
		} else if (P->Tok.is(tok::r_paren)) {
			P->ConsumeParen();
			break; // End of parameter List
		} else {
			// Handle error: Unexpected token
			P->Diag(P->Tok.getLocation(), diag::err_parser_expected_comma_or_rparen);
		}
	}

	return ASTBuilder::CreateCall(Loc, Name, Args, CallKind, Parent);
}

/**
 * ParseModule Array Value Expression
 * @return the ASTValueExpr
 */
ASTValue *ParserExpr::ParseValues() {
    FLY_DEBUG_SCOPE("Parser", "ParseValues");
    const SourceLocation &StartLoc = P->ConsumeBrace(P->BracketCount);

    // Set Values Struct and Array for next
    bool isStruct = false;
    llvm::StringMap<ASTValue *> StructValues;
    llvm::SmallVector<ASTValue *, 8> ArrayValues;

    // Parse array values Ex. {1, 2, 3}
    while(P->Tok.isNot(tok::r_brace)) {

        // if is Identifier -> struct
        if (P->Tok.isAnyIdentifier()) {
            isStruct = true;
            const StringRef &Key = P->Tok.getIdentifierInfo()->getName();
            P->ConsumeToken();

            if (P->Tok.is(tok::equal)) {
                // FIXME
                P->ConsumeToken();

                ASTValue *Value = ParseValue();
                if (Value) {
                    StructValues.insert(std::make_pair(Key, Value));
                } else {
                    P->Diag(diag::err_parser_invalid_value) << P->Tok.getName();
                }
            }
        } else { // if is Value -> array
            ASTValue *Value = ParseValue();
            if (Value) {
                ArrayValues.push_back(Value);
            } else {
                P->Diag(diag::err_parser_invalid_value) << P->Tok.getName();
            }
        }

        if (P->Tok.is(tok::comma)) {
           P->ConsumeToken();
        } else {
            break;
        }
    };

    // End of Array
    if (P->Tok.is(tok::r_brace)) {
        P->ConsumeBrace(P->BracketCount);
        if (isStruct) {
            return ASTBuilder::CreateStructValue(StartLoc, StructValues);
        } else {
            return ASTBuilder::CreateArrayValue(StartLoc, ArrayValues);
        }
    }

    P->Diag(diag::err_parser_invalid_value) << P->Tok.getName();
    return nullptr;
}
