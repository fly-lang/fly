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
        case tok::pipe:
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
            return ASTBinaryOpExprKind::OP_BINARY_AND_ASSIGN;
        case tok::star:
            return ASTBinaryOpExprKind::OP_BINARY_MUL;
        case tok::starequal:
            return ASTBinaryOpExprKind::OP_BINARY_ASSIGN_MUL;
        case tok::plus:
            return ASTBinaryOpExprKind::OP_BINARY_ADD;
        case tok::plusequal:
            return ASTBinaryOpExprKind::OP_BINARY_ADD_ASSIGN;
        case tok::minus:
            return ASTBinaryOpExprKind::OP_BINARY_SUB;
        case tok::minusequal:
            return ASTBinaryOpExprKind::OP_BINARY_SUB_ASSIGN;
        case tok::exclaimequal:
            return ASTBinaryOpExprKind::OP_BINARY_NE;
        case tok::slash:
            return ASTBinaryOpExprKind::OP_BINARY_DIV;
        case tok::slashequal:
            return ASTBinaryOpExprKind::OP_BINARY_DIV_ASSIGN;
        case tok::percent:
            return ASTBinaryOpExprKind::OP_BINARY_MOD;
        case tok::percentequal:
            return ASTBinaryOpExprKind::OP_BINARY_MOD_ASSIGN;
        case tok::less:
            return ASTBinaryOpExprKind::OP_BINARY_LT;
        case tok::lessless:
            return ASTBinaryOpExprKind::OP_BINARY_SHIFT_L;
        case tok::lessequal:
            return ASTBinaryOpExprKind::OP_BINARY_LTE;
        case tok::lesslessequal:
            return ASTBinaryOpExprKind::OP_BINARY_SHIFT_L_ASSIGN;
        case tok::greater:
            return ASTBinaryOpExprKind::OP_BINARY_GT;
        case tok::greatergreater:
            return ASTBinaryOpExprKind::OP_BINARY_SHIFT_R;
        case tok::greaterequal:
            return ASTBinaryOpExprKind::OP_BINARY_GTE;
        case tok::greatergreaterequal:
            return ASTBinaryOpExprKind::OP_BINARY_SHIFT_R_ASSIGN;
        case tok::caret:
            return ASTBinaryOpExprKind::OP_BINARY_XOR;
        case tok::caretequal:
            return ASTBinaryOpExprKind::OP_BINARY_XOR_ASSIGN;
        case tok::pipe:
            return ASTBinaryOpExprKind::OP_BINARY_OR;
        case tok::pipepipe:
            return ASTBinaryOpExprKind::OP_BINARY_OR_LOG;
        case tok::pipeequal:
            return ASTBinaryOpExprKind::OP_BINARY_OR_ASSIGN;
        case tok::equal:
            return ASTBinaryOpExprKind::OP_BINARY_ASSINGN;
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
        return ParseNewExpr(P);
    } else if (P->Tok.is(tok::l_paren)) {
        ASTExpr *Primary = Parse(P);
        if (P->Tok.isNot(tok::r_paren)) {
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

/**
 * ParseModule a statement assignment Expression
 * @param Block
 * @param VarRef
 * @param Success true on Success or false on Error
 * @return the ASTExpr
 */
//ASTExpr *ExprParser::ParseAssignExpr(ASTVarRef *VarRef) {
//    FLY_DEBUG_MESSAGE("ExprParser", "ParseAssignExpr", Logger()
//            .Attr("VarRef", VarRef).End());
//
//    // Parsing =
//    if (P->Tok.is(tok::equal)) {
//        P->ConsumeToken();
//        return ParseExpr();
//    }
//
//    // Create First Expr
//    ASTVarRefExpr *First = P->Builder.CreateExpr(VarRef);
//    Group.push_back(First);
//
//    // Parse binary assignment operator
//    ASTBinaryOperatorKind Op;
//    switch (P->Tok.getKind()) {
//
//        // Arithmetic
//        case tok::plusequal:
//            Op = ASTBinaryOperatorKind::BINARY_ARITH_ADD;
//            break;
//        case tok::minusequal:
//            Op = ASTBinaryOperatorKind::BINARY_ARITH_SUB;
//            break;
//        case tok::starequal:
//            Op = ASTBinaryOperatorKind::BINARY_ARITH_MUL;
//            break;
//        case tok::slashequal:
//            Op = ASTBinaryOperatorKind::BINARY_ARITH_DIV;
//            break;
//        case tok::percentequal:
//            Op = ASTBinaryOperatorKind::BINARY_ARITH_MOD;
//            break;
//
//            // Bit
//        case tok::ampequal:
//            Op = ASTBinaryOperatorKind::BINARY_ARITH_AND;
//            break;
//        case tok::pipeequal:
//            Op = ASTBinaryOperatorKind::BINARY_ARITH_OR;
//            break;
//        case tok::caretequal:
//            Op = ASTBinaryOperatorKind::BINARY_ARITH_XOR;
//            break;
//        case tok::lesslessequal:
//            Op = ASTBinaryOperatorKind::BINARY_ARITH_SHIFT_L;
//            break;
//        case tok::greatergreaterequal:
//            Op = ASTBinaryOperatorKind::BINARY_ARITH_SHIFT_R;
//            break;
//        default:
//            assert(0 && "Accept Only assignment operators");
//    }
//    Group.push_back(P->Builder.CreateOperatorExpr(P->ConsumeToken(), Op));
//
//    // Parse second operator
//    ASTExpr *Second = ParseExpr();
//
//    // Error: missing second operator
//    if (First == nullptr) {
//        P->Diag(P->Tok.getLocation(), diag::err_parser_miss_oper);
//        return nullptr;
//    }
//
//    // This is the last item in the expression
//    Group.push_back(Second);
//
//    // Update Group
//    UpdateBinaryGroup(false);
//    UpdateBinaryGroup(true);
//    assert(Group.size() == 1 && "Only one Group entry at the end");
//    assert(Group[0]->getExprKind() == ASTExprKind::EXPR_GROUP && "Only one Group entry at the end");
//    assert(((ASTOpExpr *) Group[0])->getOpExprKind() == ASTExprGroupKind::GROUP_BINARY && "Only one Group entry at the end");
//    return (ASTBinaryOpExpr *) Group[0];
//}

 /**
  * ParseModule all Expressions
  * @param Stmt where come from
  * @param IsFirst when starting parse expression in a binary ternary or other multi expression
  * @return the ASTExpr
  */
//ASTExpr *ExprParser::ParseExpr(bool IsFirst) {
//     FLY_DEBUG_MESSAGE("ExprParser", "ParseExpr", Logger()
//             .Attr("IsFirst", IsFirst).End());
//
//    // The parsed ASTExpr
//    ASTExpr *Expr;
//
//    // Location of the starting expression
//    if (P->Tok.is(tok::l_paren)) { // Start a new Group of Expressions
//        P->ConsumeParen();
//
//        ExprParser SubP(P);
//        Expr = SubP.ParseExpr();
//        if (Expr) {
//            if (P->Tok.is(tok::r_paren)) {
//                P->ConsumeParen();
//            } else { // Error: parenthesis unclosed
//                P->Diag(P->Tok.getLocation(), diag::err_paren_unclosed);
//                return nullptr;
//            }
//        }
//    } else if (P->isValue()) { // Ex. 1
//        Expr = P->Builder.CreateExpr(P->ParseValue());
//    } else if (P->Tok.isAnyIdentifier()) { // Ex. a or a++ or func()
//        Expr = ParseExpr(P->ParseIdentifier());
//    } else if (P->isUnaryPreOperator(P->Tok)) { // Ex. ++a or --a or !a
//        Expr = ParseUnaryPreExpr(P); // Parse Unary Post Expression
//    } else if (P->isNewOperator(P->Tok)) {
//        Expr = ParseNewExpr(P);
//    }
//
//     // Error: missing expression
//     if (Expr == nullptr) {
//         P->Diag(P->Tok.getLocation(), diag::err_parser_miss_expr);
//         return nullptr;
//     }
//
//    // Check if binary operator exists
//    if (P->isBinaryOperator()) { // Parse Binary Expression
//        Group.push_back(Expr);
//        ASTBinaryOperatorKind Operator = ParseBinaryOperator();
//        ASTBinaryOperatorExpr *BinaryOperatorExpr = P->Builder.CreateOperatorExpr(P->ConsumeToken(), Operator);
//        Group.push_back(BinaryOperatorExpr);
//        Expr = ParseExpr(false);
//
//        // Error: missing second operator
//        if (Expr == nullptr) {
//            P->Diag(P->Tok.getLocation(), diag::err_parser_miss_oper);
//            return nullptr;
//        }
//
//        if (IsFirst) {
//            // This is the last item in the expression
//            Group.push_back(Expr);
//
//            // Update Group
//            UpdateBinaryGroup(false);
//            UpdateBinaryGroup(true);
//            assert(Group.size() == 1 && "Only one Group entry at the end");
//            assert(Group[0]->getExprKind() == ASTExprKind::EXPR_GROUP && "Only one Group entry at the end");
//            assert(((ASTOpExpr *) Group[0])->getOpExprKind() == ASTExprGroupKind::GROUP_BINARY && "Only one Group entry at the end");
//            Expr = Group[0];
//        }
//    }
//    
//    if (P->isTernaryOperator() && IsFirst) { // Parse Ternary Expression
//
//        // Parse True Expr
//        ASTTernaryOperatorExpr *FirstOperator = P->Builder.CreateOperatorExpr(P->ConsumeToken(),
//                                                                              ASTTernaryOpExprKind::TERNARY_IF);
//
//        ExprParser SubSecond(P);
//        ASTExpr *FirstExpr = SubSecond.ParseExpr();
//
//        if (P->Tok.getKind() == tok::colon) {
//            ASTTernaryOperatorExpr *SecondOperator = P->Builder.CreateOperatorExpr(P->ConsumeToken(),
//                                                                                   ASTTernaryOpExprKind::TERNARY_ELSE);
//
//            ExprParser SubThird(P);
//            ASTExpr *SecondExpr = SubThird.ParseExpr();
//            if (SecondExpr != nullptr)
//                return P->Builder.CreateTernaryExpr(Expr, FirstOperator, FirstExpr, SecondOperator, SecondExpr);
//        }
//
//        // Error: Invalid operator in Ternary condition
//        P->Diag(P->Tok.getLocation(), diag::err_parser_miss_oper);
//        return nullptr;
//    }
//
//    return Expr;
//}
//
//ASTExpr *ExprParser::ParseExpr(ASTIdentifier *Identifier) {
//    FLY_DEBUG_MESSAGE("ExprParser", "ParseExpr",
//                      Logger().Attr("Identifier", Identifier).End());
//    if (Identifier->isCall()) { // Ex. a()
//        ASTCallExpr *CallExpr = P->Builder.CreateExpr((ASTCall *) Identifier);
//        return CallExpr;
//    } else { // parse variable post increment/decrement or simple var
//        ASTVarRef *VarRef = P->Builder.CreateVarRef(Identifier);
//        if (P->isUnaryPostOperator()) { // Ex. a++ or a--
//            return ParseUnaryPostExpr(VarRef); // Parse Unary Pre Expression
//        } else {
//            // Simple Var
//            ASTVarRefExpr *VarRefExpr = P->Builder.CreateExpr(VarRef);
//            return VarRefExpr;
//        }
//    }
//}

ASTExpr *ParserExpr::ParseNewExpr(Parser *P) {
    FLY_DEBUG("ExprParser", "ParseNewExpr"); // TODO add assert(keyword is new)
    P->ConsumeToken();
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
