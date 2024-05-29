//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/ExprParser.cpp - Expression Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/ExprParser.h"
#include "AST/ASTVar.h"
#include "AST/ASTVarRef.h"
#include "AST/ASTIdentifier.h"
#include "AST/ASTStmt.h"
#include "AST/ASTOperatorExpr.h"
#include "Sema/SemaBuilder.h"
#include "Basic/Debug.h"

using namespace fly;

ExprParser::ExprParser(Parser *P) : P(P) {
    FLY_DEBUG_MESSAGE("ExprParser", "ExprParser", Logger().End());
}

/**
 * Parse a statement assignment Expression
 * @param Block
 * @param VarRef
 * @param Success true on Success or false on Error
 * @return the ASTExpr
 */
ASTExpr *ExprParser::ParseAssignExpr(ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("ExprParser", "ParseAssignExpr", Logger()
            .Attr("VarRef", VarRef).End());

    // Parsing =
    if (P->Tok.is(tok::equal)) {
        P->ConsumeToken();
        return ParseExpr();
    }

    // Create First Expr
    ASTVarRefExpr *First = P->Builder.CreateExpr(VarRef);
    Group.push_back(First);

    // Parse binary assignment operator
    ASTBinaryOperatorKind Op;
    switch (P->Tok.getKind()) {

        // Arithmetic
        case tok::plusequal:
            Op = ASTBinaryOperatorKind::BINARY_ARITH_ADD;
            break;
        case tok::minusequal:
            Op = ASTBinaryOperatorKind::BINARY_ARITH_SUB;
            break;
        case tok::starequal:
            Op = ASTBinaryOperatorKind::BINARY_ARITH_MUL;
            break;
        case tok::slashequal:
            Op = ASTBinaryOperatorKind::BINARY_ARITH_DIV;
            break;
        case tok::percentequal:
            Op = ASTBinaryOperatorKind::BINARY_ARITH_MOD;
            break;

            // Bit
        case tok::ampequal:
            Op = ASTBinaryOperatorKind::BINARY_ARITH_AND;
            break;
        case tok::pipeequal:
            Op = ASTBinaryOperatorKind::BINARY_ARITH_OR;
            break;
        case tok::caretequal:
            Op = ASTBinaryOperatorKind::BINARY_ARITH_XOR;
            break;
        case tok::lesslessequal:
            Op = ASTBinaryOperatorKind::BINARY_ARITH_SHIFT_L;
            break;
        case tok::greatergreaterequal:
            Op = ASTBinaryOperatorKind::BINARY_ARITH_SHIFT_R;
            break;
        default:
            assert(0 && "Accept Only assignment operators");
    }
    Group.push_back(P->Builder.CreateOperatorExpr(P->ConsumeToken(), Op));

    // Parse second operator
    ASTExpr *Second = ParseExpr();

    // Error: missing second operator
    if (First == nullptr) {
        P->Diag(P->Tok.getLocation(), diag::err_parser_miss_oper);
        return nullptr;
    }

    // This is the last item in the expression
    Group.push_back(Second);

    // Update Group
    UpdateBinaryGroup(false);
    UpdateBinaryGroup(true);
    assert(Group.size() == 1 && "Only one Group entry at the end");
    assert(Group[0]->getExprKind() == ASTExprKind::EXPR_GROUP && "Only one Group entry at the end");
    assert(((ASTGroupExpr *) Group[0])->getGroupKind() == ASTExprGroupKind::GROUP_BINARY && "Only one Group entry at the end");
    return (ASTBinaryGroupExpr *) Group[0];
}

 /**
  * Parse all Expressions
  * @param Stmt where come from
  * @param IsFirst when starting parse expression in a binary ternary or other multi expression
  * @return the ASTExpr
  */
ASTExpr *ExprParser::ParseExpr(bool IsFirst) {
     FLY_DEBUG_MESSAGE("ExprParser", "ParseExpr", Logger()
             .Attr("IsFirst", IsFirst).End());

    // The parsed ASTExpr
    ASTExpr *Expr = nullptr;

    // Location of the starting expression
    if (P->Tok.is(tok::l_paren)) { // Start a new Group of Expressions
        P->ConsumeParen();

        ExprParser SubP(P);
        Expr = SubP.ParseExpr();
        if (Expr) {
            if (P->Tok.is(tok::r_paren)) {
                P->ConsumeParen();
            } else { // Error: parenthesis unclosed
                P->Diag(P->Tok.getLocation(), diag::err_paren_unclosed);
                return nullptr;
            }
        }
    } else if (P->isValue()) { // Ex. 1
        Expr = P->Builder.CreateExpr(P->ParseValue());
    } else if (P->Tok.isAnyIdentifier()) { // Ex. a or a++ or func()
        Expr = ParseExpr(P->ParseIdentifier());
    } else if (P->isUnaryPreOperator(P->Tok)) { // Ex. ++a or --a or !a
        Expr = ParseUnaryPreExpr(P); // Parse Unary Post Expression
    } else if (P->isNewOperator(P->Tok)) {
        Expr = ParseNewExpr(P);
    } else {
        // FIXME? remove or change logic?
        // Used with: return
        Expr =  P->Builder.CreateExpr(); // return an ASTEmptyExpr
        return Expr;
    }

     // Error: missing expression
     if (Expr == nullptr) {
         P->Diag(P->Tok.getLocation(), diag::err_parser_miss_expr);
         return nullptr;
     }

    // Check if binary operator exists
    if (P->isBinaryOperator()) { // Parse Binary Expression
        Group.push_back(Expr);
        ASTBinaryOperatorKind Operator = ParseBinaryOperator();
        ASTBinaryOperatorExpr *BinaryOperatorExpr = P->Builder.CreateOperatorExpr(P->ConsumeToken(), Operator);
        Group.push_back(BinaryOperatorExpr);
        Expr = ParseExpr(false);

        // Error: missing second operator
        if (Expr == nullptr) {
            P->Diag(P->Tok.getLocation(), diag::err_parser_miss_oper);
            return nullptr;
        }

        if (IsFirst) {
            // This is the last item in the expression
            Group.push_back(Expr);

            // Update Group
            UpdateBinaryGroup(false);
            UpdateBinaryGroup(true);
            assert(Group.size() == 1 && "Only one Group entry at the end");
            assert(Group[0]->getExprKind() == ASTExprKind::EXPR_GROUP && "Only one Group entry at the end");
            assert(((ASTGroupExpr *) Group[0])->getGroupKind() == ASTExprGroupKind::GROUP_BINARY && "Only one Group entry at the end");
            Expr = Group[0];
        }
    }
    
    if (P->isTernaryOperator() && IsFirst) { // Parse Ternary Expression

        // Parse True Expr
        ASTTernaryOperatorExpr *FirstOperator = P->Builder.CreateOperatorExpr(P->ConsumeToken(),
                                                                               ASTTernaryOperatorKind::TERNARY_IF);

        ExprParser SubSecond(P);
        ASTExpr *FirstExpr = SubSecond.ParseExpr();

        if (P->Tok.getKind() == tok::colon) {
            ASTTernaryOperatorExpr *SecondOperator = P->Builder.CreateOperatorExpr(P->ConsumeToken(),
                                                                                    ASTTernaryOperatorKind::TERNARY_ELSE);

            ExprParser SubThird(P);
            ASTExpr *SecondExpr = SubThird.ParseExpr();
            if (SecondExpr != nullptr)
                return P->Builder.CreateTernaryExpr(Expr, FirstOperator, FirstExpr, SecondOperator, SecondExpr);
        }

        // Error: Invalid operator in Ternary condition
        P->Diag(P->Tok.getLocation(), diag::err_parser_miss_oper);
        return nullptr;
    }

    return Expr;
}

ASTExpr *ExprParser::ParseExpr(ASTIdentifier *Identifier) {
    FLY_DEBUG_MESSAGE("ExprParser", "ParseExpr",
                      Logger().Attr("Identifier", Identifier).End());
    if (Identifier->isCall()) { // Ex. a()
        ASTCallExpr *CallExpr = P->Builder.CreateExpr((ASTCall *) Identifier);
        return CallExpr;
    } else { // parse variable post increment/decrement or simple var
        ASTVarRef *VarRef = P->Builder.CreateVarRef(Identifier);
        if (P->isUnaryPostOperator()) { // Ex. a++ or a--
            return ParseUnaryPostExpr(VarRef); // Parse Unary Pre Expression
        } else {
            // Simple Var
            ASTVarRefExpr *VarRefExpr = P->Builder.CreateExpr(VarRef);
            return VarRefExpr;
        }
    }
}

ASTExpr *ExprParser::ParseNewExpr(Parser *P) {
    FLY_DEBUG("ExprParser", "ParseNewExpr"); // TODO add assert(keyword is new)
    P->ConsumeToken();
    if (P->Tok.isAnyIdentifier()) {
        ASTIdentifier *Identifier = P->ParseIdentifier();

        if (Identifier->isCall()) { // Ex. a()
            ASTCallExpr *CallExpr = P->Builder.CreateNewExpr((ASTCall *) Identifier);
            return CallExpr;
        }
    }

    // Error:
    P->Diag(P->Tok.getLocation(), diag::err_parse_new_instance);
    return nullptr;
}

/**
 * Parse unary pre operators ++a, --a, !a
 * @param Block
 * @param VarRef
 * @param Success
 * @return
 */
ASTUnaryGroupExpr *ExprParser::ParseUnaryPostExpr(ASTVarRef *VarRef) {
    FLY_DEBUG_MESSAGE("ExprParser", "ParseUnaryPostExpr", Logger()
            .Attr("VarRef", VarRef).End());
    ASTVarRefExpr *VarRefExpr = P->Builder.CreateExpr(VarRef);
    ASTUnaryOperatorKind OperatorKind;
    switch (P->Tok.getKind()) {
        case tok::plusplus:
            OperatorKind = ASTUnaryOperatorKind::UNARY_ARITH_POST_INCR;
            break;
        case tok::minusminus:
            OperatorKind = ASTUnaryOperatorKind::UNARY_ARITH_POST_DECR;
            break;
        default:
            // Error:
            return nullptr;
    }
    P->ConsumeToken();
    ASTUnaryOperatorExpr *OperatorExpr = P->Builder.CreateOperatorExpr(VarRef->getLocation(), OperatorKind);
    return P->Builder.CreateUnaryExpr(OperatorExpr, VarRefExpr);
}

/**
 * Parse unary operators ++a, --a, !a
 * @param Block
 * @param Success
 * @return
 */
ASTUnaryGroupExpr* ExprParser::ParseUnaryPreExpr(Parser *P) {
    FLY_DEBUG("ExprParser", "ParseUnaryPreExpr");

    ASTUnaryOperatorKind Op;
    switch (P->Tok.getKind()) {
        case tok::exclaim:
            Op = ASTUnaryOperatorKind::UNARY_LOGIC_NOT;
            break;
        case tok::plusplus:
            Op = ASTUnaryOperatorKind::UNARY_ARITH_PRE_INCR;
            break;
        case tok::minusminus:
            Op = ASTUnaryOperatorKind::UNARY_ARITH_PRE_DECR;
            break;
        default:
            // Error:
            return nullptr;
    }
    const SourceLocation &Loc = P->ConsumeToken();

    // Check var identifier
    if (P->Tok.isAnyIdentifier()) {
        ASTIdentifier *Identifier = P->ParseIdentifier();
        ASTVarRef *VarRef = P->Builder.CreateVarRef(Identifier);
        ASTVarRefExpr *VarRefExpr = P->Builder.CreateExpr(VarRef);
        ASTUnaryOperatorExpr *OperatorExpr = P->Builder.CreateOperatorExpr(Loc, Op);
        return P->Builder.CreateUnaryExpr(OperatorExpr, VarRefExpr);
    }

    // Error: unary operator
    P->Diag(P->Tok.getLocation(), diag::err_unary_operator) << getPunctuatorSpelling(P->Tok.getKind());
    return nullptr;
}

/**
 * Parse Binary operators
 * @param Block
 * @param First
 * @param Success
 * @return
 */
ASTBinaryOperatorKind ExprParser::ParseBinaryOperator() {
    FLY_DEBUG("ExprParser", "ParseBinaryOperator");

    ASTBinaryOperatorKind Op;
    switch (P->Tok.getKind()) {

        // Binary Arithmetic
        case tok::plus:
            return ASTBinaryOperatorKind::BINARY_ARITH_ADD;
        case tok::minus:
            return ASTBinaryOperatorKind::BINARY_ARITH_SUB;
        case tok::star:
            return ASTBinaryOperatorKind::BINARY_ARITH_MUL;
        case tok::slash:
            return ASTBinaryOperatorKind::BINARY_ARITH_DIV;
        case tok::percent:
            return ASTBinaryOperatorKind::BINARY_ARITH_MOD;
        case tok::amp:
            return ASTBinaryOperatorKind::BINARY_ARITH_AND;
        case tok::pipe:
            return ASTBinaryOperatorKind::BINARY_ARITH_OR;
        case tok::caret:
            return ASTBinaryOperatorKind::BINARY_ARITH_XOR;
        case tok::lessless:
            return ASTBinaryOperatorKind::BINARY_ARITH_SHIFT_L;
        case tok::greatergreater:
            return ASTBinaryOperatorKind::BINARY_ARITH_SHIFT_R;

            // Logic
        case tok::ampamp:
            return ASTBinaryOperatorKind::BINARY_LOGIC_AND;
        case tok::pipepipe:
            return ASTBinaryOperatorKind::BINARY_LOGIC_OR;

            // Comparator
        case tok::less:
            return ASTBinaryOperatorKind::BINARY_COMP_LT;
        case tok::lessequal:
            return ASTBinaryOperatorKind::BINARY_COMP_LTE;
        case tok::greater:
            return ASTBinaryOperatorKind::BINARY_COMP_GT;
        case tok::greaterequal:
            return ASTBinaryOperatorKind::BINARY_COMP_GTE;
        case tok::exclaimequal:
            return ASTBinaryOperatorKind::BINARY_COMP_NE;
        case tok::equalequal:
            return ASTBinaryOperatorKind::BINARY_COMP_EQ;

    }
    assert(0 && "Binary Operator not accepted");
}

void ExprParser::UpdateBinaryGroup(bool NoPrecedence) {
    FLY_DEBUG_MESSAGE("ClassParser", "ParseMethod", Logger()
            .Attr("NoPrecedence", NoPrecedence).End());
    std::vector<ASTExpr *> Result;
    ASTBinaryOperatorExpr *Op;
    ASTExpr *Second;
    std::vector<ASTExpr *>::const_iterator It = Group.begin();
    ASTExpr *First = *It;
    Result.push_back(First);
    // Algorithm: take the first, and after by 2
    // a + b / c * d
    //|_|_ _|_ _|_ _|
    // 0 1 2 3 4 5 6
    while (It != Group.end()) {

        // The operation
        if (++It != Group.end()) {
            Op = (ASTBinaryOperatorExpr *) *It;

            // The second operator
            if (++It != Group.end()) {
                Second = *It;
                if (NoPrecedence || Op->isPrecedence()) {
                    First = Result.back();
                    Result.pop_back();

                    Result.push_back(P->Builder.CreateBinaryExpr(Op, First, Second));
                } else {
                    Result.push_back(Op);
                    Result.push_back(Second);
                }
            } else {
                // Error:
                P->Diag(P->Tok.getLocation(), diag::err_parser_miss_oper);
                return;
            }
        }
    }
    Group = Result;
}
