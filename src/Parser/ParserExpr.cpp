//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/ExprParser.cpp - Expression Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/ParserExpr.h"
#include "AST/ASTVar.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTStmt.h"
#include "AST/ASTOpExpr.h"
#include "Sema/SemaBuilder.h"
#include "Basic/Debug.h"

using namespace fly;

Precedence getPrecedence(Token Tok) {
    switch (Tok.getKind()) {
        case fly::tok::question:
            return Precedence::TERNARY;
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
    // Only assignment operators are right-associative
    return Tok.isOneOf(tok::plusequal, tok::minusequal, tok::starequal, tok::slashequal, tok::percentequal,
                       tok::ampequal, tok::pipeequal, tok::caretequal, tok::lesslessequal, tok::greatergreaterequal);
}

ASTUnaryOpExprKind toUnaryOpExprKind(Token Tok, bool isPost) {
    if (isPost) {
        switch (Tok.getKind()) {
            case tok::plusplus:
                return ASTUnaryOpExprKind::OP_UNARY_POST_INCR;
            case tok::minusminus:
                return ASTUnaryOpExprKind::OP_UNARY_POST_DECR;
        }
    } else {
        switch (Tok.getKind()) {
            case tok::plusplus:
                return ASTUnaryOpExprKind::OP_UNARY_PRE_INCR;
            case tok::minusminus:
                return ASTUnaryOpExprKind::OP_UNARY_PRE_DECR;
            case tok::exclaim:
                return ASTUnaryOpExprKind::OP_UNARY_PRE_DECR;
        }
    }
    assert(false && "Invalid Unary Token details");
}

ASTBinaryOpExprKind toBinaryOpExprKind(Token Tok) {
    switch (Tok.getKind()) {
        case tok::amp:
            return ASTBinaryOpExprKind::OP_BINARY_AND;
        case tok::ampamp:
            return ASTBinaryOpExprKind::OP_BINARY_AND_LOG;
        case tok::ampequal:
            return ASTBinaryOpExprKind::OP_BINARY_ASSIGN_AND;
        case tok::star:
            return ASTBinaryOpExprKind::OP_BINARY_MUL;
        case tok::starequal:
            return ASTBinaryOpExprKind::OP_BINARY_ASSIGN_MUL;
        case tok::plus:
            return ASTBinaryOpExprKind::OP_BINARY_ADD;
        case tok::plusequal:
            return ASTBinaryOpExprKind::OP_BINARY_ASSIGN_ADD;
        case tok::minus:
            return ASTBinaryOpExprKind::OP_BINARY_SUB;
        case tok::minusequal:
            return ASTBinaryOpExprKind::OP_BINARY_ASSIGN_SUB;
        case tok::exclaimequal:
            return ASTBinaryOpExprKind::OP_BINARY_NE;
        case tok::slash:
            return ASTBinaryOpExprKind::OP_BINARY_DIV;
        case tok::slashequal:
            return ASTBinaryOpExprKind::OP_BINARY_ASSIGN_DIV;
        case tok::percent:
            return ASTBinaryOpExprKind::OP_BINARY_MOD;
        case tok::percentequal:
            return ASTBinaryOpExprKind::OP_BINARY_ASSIGN_MOD;
        case tok::less:
            return ASTBinaryOpExprKind::OP_BINARY_LT;
        case tok::lessless:
            return ASTBinaryOpExprKind::OP_BINARY_SHIFT_L;
        case tok::lessequal:
            return ASTBinaryOpExprKind::OP_BINARY_LTE;
        case tok::lesslessequal:
            return ASTBinaryOpExprKind::OP_BINARY_ASSIGN_SHIFT_L;
        case tok::greater:
            return ASTBinaryOpExprKind::OP_BINARY_GT;
        case tok::greatergreater:
            return ASTBinaryOpExprKind::OP_BINARY_SHIFT_R;
        case tok::greaterequal:
            return ASTBinaryOpExprKind::OP_BINARY_GTE;
        case tok::greatergreaterequal:
            return ASTBinaryOpExprKind::OP_BINARY_ASSIGN_SHIFT_R;
        case tok::caret:
            return ASTBinaryOpExprKind::OP_BINARY_XOR;
        case tok::caretequal:
            return ASTBinaryOpExprKind::OP_BINARY_ASSIGN_XOR;
        case tok::pipe:
            return ASTBinaryOpExprKind::OP_BINARY_OR;
        case tok::pipepipe:
            return ASTBinaryOpExprKind::OP_BINARY_OR_LOG;
        case tok::pipeequal:
            return ASTBinaryOpExprKind::OP_BINARY_ASSIGN_OR;
        case tok::equal:
            return ASTBinaryOpExprKind::OP_BINARY_ASSIGN;
        case tok::equalequal:
            return ASTBinaryOpExprKind::OP_BINARY_EQ;
    }
    assert(false && "Invalid Binary Token details");
}

ParserExpr::ParserExpr(Parser *P, ASTExpr *Left) : P(P) {
    FLY_DEBUG_MESSAGE("ExprParser", "ExprParser", Logger().End());

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
    Expr = Left;
}

ASTExpr *ParserExpr::Parse(Parser *P, ASTExpr *Expr) {
    ParserExpr *EP = new ParserExpr(P, Expr);
    return EP->Expr;
}

ASTExpr *ParserExpr::ParsePrimary() {
    Token &Tok = P->Tok;

    if (P->isValue()) { // Ex. 1
        return P->Builder.CreateExpr(P->ParseValue());
    } else if (P->Tok.isAnyIdentifier()) { // Ex. a or a++ or func()
        ASTIdentifier *Identifier = P->ParseIdentifier();
        if (Identifier->isCall()) { // Ex. a()
            return P->Builder.CreateExpr((ASTCall *) Identifier);
        } else { // parse function call, variable post increment/decrement or simple var
            ASTVarRef *VarRef = P->Builder.CreateVarRef(Identifier);
            ASTVarRefExpr *Primary = P->Builder.CreateExpr(VarRef);
            if (isUnaryPostOperator()) { // Ex. a++ or a--
                ASTUnaryOpExprKind OpKind = toUnaryOpExprKind(Tok, true);
                P->ConsumeToken();
                return P->Builder.CreateUnaryOpExpr(Primary->getLocation(), OpKind, Primary);
            } else {
                // Simple VarRef
                return P->Builder.CreateExpr(VarRef);
            }
        }
    } else if (isUnaryPreOperator(P->Tok)) { // Ex. ++a or --a or !a
        ASTUnaryOpExprKind OpKind = toUnaryOpExprKind(Tok, false);
        const SourceLocation &Loc = P->ConsumeToken();
        ASTExpr* Primary = ParsePrimary();  // Parse the operand (recursively)
        return P->Builder.CreateUnaryOpExpr(Loc, OpKind, Primary);
    } else if (isNewOperator(P->Tok)) {
        return ParseNewExpr();
    } else if (P->Tok.is(tok::l_paren)) {
        P->ConsumeParen();
        ASTExpr *Primary = Parse(P);
        if (P->Tok.is(tok::r_paren)) {
            P->ConsumeParen();
        } else {
            P->Diag(P->Tok.getLocation(), diag::err_parse_expr_close_paren);
        }
        return Primary;
    }

    P->Diag(P->Tok.getLocation(), diag::err_parse_expr_expected_primary);
    return nullptr;
}

ASTUnaryOpExpr *ParserExpr::ParseUnaryExpr() {
    Token OpTok = P->Tok;

    // Parse the operand
    ASTExpr* Primary = ParsePrimary();

    // Return the unary operation AST node
    return P->Builder.CreateUnaryOpExpr(OpTok.getLocation(), toUnaryOpExprKind(OpTok, true), Primary);
}

ASTBinaryOpExpr *ParserExpr::ParseBinaryExpr(ASTExpr *LeftExpr, Token OpToken, Precedence precedence) {
    // Consume the binary operator
    P->ConsumeToken();

    // Parse the right-hand side of the binary expression
    ASTExpr* RightExpr = ParsePrimary();  // Parse the RHS (which may include parentheses)

    // Check for higher precedence operators on the right-hand side
    Token NextTok = P->Tok;
    Precedence nextPrecedence = getPrecedence(NextTok);

    if (nextPrecedence > precedence ||
        (nextPrecedence == precedence && isRightAssociative(OpToken))) {
        RightExpr = ParseBinaryExpr(RightExpr, NextTok, nextPrecedence);  // Continue climbing
    }

    // Combine the left and right into a binary operation node

    return P->Builder.CreateBinaryOpExpr(toBinaryOpExprKind(OpToken), OpToken.getLocation(), LeftExpr, RightExpr);
}

ASTTernaryOpExpr *ParserExpr::ParseTernaryExpr(ASTExpr *ConditionExpr) {
    const SourceLocation &TrueOpLoc = P->ConsumeToken();  // Consume '?'

    ASTExpr* TrueExpr = Parse(P);  // Parse the true expression

    if (P->Tok.isNot(tok::colon)) {
        throw P->Diag(P->Tok.getLocation(), diag::err_parse_ternary_expr);
    }

    const SourceLocation &FalseOpLoc = P->ConsumeToken();  // Consume ':'

    ASTExpr* FalseExpr = Parse(P);  // Parse the false expression

    return P->Builder.CreateTernaryOpExpr(ConditionExpr, TrueOpLoc, TrueExpr, FalseOpLoc, FalseExpr);
}

/**
 * Check if Token is one of the Unary Pre Operators
 * @return true on Success or false on Error
 */
bool ParserExpr::isNewOperator(Token &Tok) {
    FLY_DEBUG("Parser", "isNewOperator");
    return Tok.is(tok::kw_new);
}

/**
 * Check if Token is one of the Unary Pre Operators
 * @return true on Success or false on Error
 */
bool ParserExpr::isUnaryPreOperator(Token &Tok) {
    FLY_DEBUG("Parser", "isUnaryPreOperator");
    return Tok.isOneOf(tok::plusplus, tok::minusminus, tok::exclaim);
}

/**
 * Check if Token is one of the Unary Post Operators
 * @return true on Success or false on Error
 */
bool ParserExpr::isUnaryPostOperator() {
    FLY_DEBUG("Parser", "isUnaryPostOperator");
    return P->Tok.isOneOf(tok::plusplus, tok::minusminus);
}

/**
 * Check if Token is one of Binary Operators
 * @return true on Success or false on Error
 */
bool ParserExpr::isBinaryOperator() {
    FLY_DEBUG("Parser", "isBinaryOperator");
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
    FLY_DEBUG("Parser", "isTernaryOperator");
    return P->Tok.is(tok::question);
}

ASTExpr *ParserExpr::ParseNewExpr() {
    FLY_DEBUG("ExprParser", "ParseNewExpr");
    const SourceLocation &NewOpLoc = P->ConsumeToken();  // Consume 'new'

    if (P->Tok.isAnyIdentifier()) {
        ASTIdentifier *Identifier = P->ParseIdentifier();

        if (Identifier->isCall()) { // Ex. a()
            ASTCallExpr *CallExpr = P->Builder.CreateExpr((ASTCall *) Identifier);
            return CallExpr;
        }
    }

    // Error:
    P->Diag(P->Tok.getLocation(), diag::err_parse_new_instance);
    return nullptr;
}
