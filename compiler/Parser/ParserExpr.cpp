//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Parser/ParserExpr.cpp - expression parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/ParserExpr.h"

#include "AST/ASTBinary.h"
#include "AST/ASTBuilder.h"
#include "AST/ASTCast.h"
#include "AST/ASTType.h"
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
            case tok::minus:
                return ASTUnaryKind::OP_UNARY_NEG;   // B007: unary minus
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

	// Expr contains a binary or ternary operator.
	// A null Left means the primary failed to parse (already diagnosed): an
	// operator must not be applied to it — building an ASTBinary/ASTTernary
	// with a null operand crashed Sema after the diagnostic.
	if (Left != nullptr && (isBinaryOperator() || isTernaryOperator())) {

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

			// An operand failed to parse: stop chaining operators onto null
			if (Left == nullptr) {
				break;
			}
		}
	}

	return Left;
}


ASTExpr * ParserExpr::ParseIdentifierOrCall(ASTExpr *Parent) {
	// Guard: keywords after '.' are allowed (they carry an IdentifierInfo), but
	// a token with NO identifier info (number, punctuation) crashed here.
	if (!P->Tok.getIdentifierInfo()) {
		P->Diag(P->Tok.getLocation(), diag::err_parser_identifier_expected);
		return nullptr;
	}
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

	// `error` in EXPRESSION position reads the implicit error variable a bare
	// `handle { }` binds (B031, self-host parity): the keyword maps to the
	// internal name "__error" the handle registered (a non-keyword name no
	// user identifier can collide with).
	if (P->Tok.is(tok::kw_error)) {
		const SourceLocation &ErrLoc = P->ConsumeToken();
		return ASTBuilder::CreateIdentifier(ErrLoc, "__error");
	}

	// Parse Identifier or Call
	if (P->Tok.isAnyIdentifier()) { // Ex. a or a++ or func()

		ASTExpr *Primary = ParseIdentifierOrCall();
		if (Primary == nullptr)
			return nullptr;

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
		// Operand failed to parse (already diagnosed): don't build a unary on null
		if (Primary == nullptr)
			return nullptr;
		return ASTBuilder::CreateUnary(OpLoc, OpKind, Primary);
	}

	// Parse New Expression
	if (isNewOperator(P->Tok)) {
		return ParseNewExpr();
	}

	// Parse Parentheses — either a C-style cast "(<builtin-type>) expr" or a
	// plain parenthesized expression. A builtin-type keyword cannot begin an
	// expression, so "( <builtin-type>" is an unambiguous cast.
	if (P->Tok.is(tok::l_paren)) {
		P->ConsumeParen();

		// C-style cast to a builtin (numeric) type: (int)x, (uint)u, (byte)l, …
		if (P->isBuiltinType(P->Tok)) {
			ASTType *ToType = P->ParseType();
			if (P->Tok.is(tok::r_paren)) {
				P->ConsumeParen();
			} else {
				P->Diag(P->Tok.getLocation(), diag::err_parser_expr_close_paren);
			}
			ASTExpr *Operand = ParsePrimary();   // cast binds tightly to the operand
			// Operand failed to parse (already diagnosed): don't build a cast on null
			if (Operand == nullptr)
				return nullptr;
			return ASTBuilder::CreateCast(Operand, ToType);
		}

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

    // RHS failed to parse (already diagnosed): an ASTBinary with a null operand
    // crashed Sema after the diagnostic — propagate the failure instead.
    if (RightExpr == nullptr)
        return nullptr;

    // Keep climbing the RHS while the next operator binds tighter than the current one.
    // A single `if` only absorbed one level; the `while` handles chains like
    // `x = a * a + b * b` where both `*` and `+` have higher precedence than `=`.
    while (true) {
        Token NextTok = P->Tok;
        Precedence nextPrecedence = getPrecedence(NextTok);
        if (nextPrecedence == Precedence::LOWEST) break;
        if (nextPrecedence == Precedence::TERNARY) {
            // '?' binds TIGHTER than assignment (and only assignment): in
            // `r = c ? 1 : 2` the ternary must fold into the '=' RHS here —
            // deferring it to the outer ParseExpr loop wrapped the finished
            // assignment as the ternary CONDITION ((r = c) ? 1 : 2), which
            // Sema then rejected as "cannot assign a value of type 'bool'"
            // for every value-producing ternary. For operators that bind
            // tighter than '?' (`a + b ? …`) the outer loop still takes it,
            // so the whole binary becomes the condition (flat table).
            if (precedence < Precedence::TERNARY) {
                RightExpr = ParseTernaryExpr(RightExpr);
                if (RightExpr == nullptr)
                    return nullptr;
                continue;
            }
            break;
        }
        if (!(nextPrecedence > precedence ||
              (nextPrecedence == precedence && isRightAssociative(OpToken)))) break;
        RightExpr = ParseBinaryExpr(RightExpr, NextTok, nextPrecedence);
        if (RightExpr == nullptr)
            return nullptr;
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
        // Was `throw P->Diag(...)`: the only throw in the compiler with NO
        // matching catch anywhere — a malformed ternary killed the process via
        // std::terminate (0xE06D7363). Diagnose and recover instead.
        P->Diag(P->Tok.getLocation(), diag::err_parser_ternary_expr);
        return nullptr;
    }

    const SourceLocation &FalseOpLoc = P->ConsumeToken();  // Consume ':'

	ParserExpr PEF(P);
    ASTExpr* FalseExpr = PEF.Parse();  // Parse the false expression

    // Either branch failed to parse (already diagnosed): an ASTTernary with a
    // null branch crashed Sema after the diagnostic — propagate the failure.
    if (TrueExpr == nullptr || FalseExpr == nullptr)
        return nullptr;

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
    // tok::minus only reaches here in OPERAND position (binary `a - b` is
    // consumed by the precedence loop after a left operand) — B007 unary minus.
    return Tok.isOneOf(tok::plusplus, tok::minusminus, tok::exclaim, tok::minus);
}

/**
 * Check if Token is one of the Unary Post Operators
 * @return true on Success or false on Error
 */
bool ParserExpr::isUnaryPostOperator() {
    FLY_DEBUG_SCOPE("ParserExpr", "isUnaryPostOperator");
    // Newlines are insignificant, so a `++`/`--` opening the NEXT line is a
    // prefix statement of its own — it must never be eaten as the previous
    // operand's postfix (B014 parity: `int a = 5` + `++a` parsed as `5++`).
    if (P->Tok.isAtStartOfLine())
        return false;
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
    	// Parse optional generic type arguments: new List<int>()
    	llvm::SmallVector<ASTType *, 4> TypeArgs;
    	if (P->Tok.is(tok::less)) {
    	    TypeArgs = P->ParseTypeArguments();
    	}
    	ASTCall *Call = ParseCall(Loc, Name, ASTCallKind::CALL_NEW);
    	if (Call && !TypeArgs.empty()) {
    	    Call->TypeArgs = std::move(TypeArgs);
    	}
    	return Call;
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

		// A truncated file must not spin waiting for ')'.
		if (P->Tok.is(tok::eof)) {
			P->Diag(P->Tok.getLocation(), diag::err_parser_expected_comma_or_rparen);
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
			// Handle error: Unexpected token. MUST bail out: staying in the loop
			// without consuming re-diagnosed the same token forever (the parser
			// hung compiling a malformed argument list).
			P->Diag(P->Tok.getLocation(), diag::err_parser_expected_comma_or_rparen);
			break;
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
    llvm::StringMap<ASTExpr *> StructValues;
    llvm::SmallVector<ASTExpr *, 8> ArrayValues;

    // Parse array values Ex. {1, 2, 3}. Array elements are full EXPRESSIONS —
    // `{(byte)200, a, x + 1}` — not just bare literals (B026): the old
    // ParseValue-only path rejected a cast with "unexpected token 'l_paren'"
    // and silently SWALLOWED identifier elements (mistaken for struct keys).
    while(P->Tok.isNot(tok::r_brace)) {

        if (P->Tok.isAnyIdentifier()) {
            // `ident =` starts a STRUCT literal entry; a bare identifier is an
            // array ELEMENT expression (continued via Parse(Left) so `a + 1`
            // and friends work).
            const StringRef &Key = P->Tok.getIdentifierInfo()->getName();
            const SourceLocation &IdLoc = P->Tok.getLocation();
            P->ConsumeToken();

            if (P->Tok.is(tok::equal)) {
                isStruct = true;
                P->ConsumeToken();

                // The field VALUE is a full expression, exactly like an array
                // element (B029 parity) — `{x = base + 1, y = (byte)n}`. A
                // nested `{ … }` recurses through ParsePrimary's ParseValues.
                ParserExpr PE(P);
                ASTExpr *Value = PE.Parse();
                if (Value) {
                    StructValues.insert(std::make_pair(Key, Value));
                } else {
                    P->Diag(diag::err_parser_invalid_value) << P->Tok.getName();
                }
            } else {
                ASTExpr *IdExpr = ASTBuilder::CreateIdentifier(IdLoc, Key);
                ParserExpr PE(P);
                ASTExpr *Elem = PE.Parse(IdExpr);
                if (Elem) {
                    ArrayValues.push_back(Elem);
                } else {
                    P->Diag(diag::err_parser_invalid_value) << P->Tok.getName();
                }
            }
        } else if (P->Tok.is(tok::l_brace)) {
            // nested array/struct literal element
            ASTValue *Nested = ParseValues();
            if (Nested) {
                ArrayValues.push_back(Nested);
            } else {
                P->Diag(diag::err_parser_invalid_value) << P->Tok.getName();
            }
        } else { // any other expression element (literal, cast, parenthesized, …)
            ParserExpr PE(P);
            ASTExpr *Elem = PE.Parse();
            if (Elem) {
                ArrayValues.push_back(Elem);
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
