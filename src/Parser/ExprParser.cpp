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
#include "Sema/SemaBuilder.h"
#include "Basic/Debug.h"

using namespace fly;

ExprParser::ExprParser(Parser *P, ASTStmt *Stmt) : P(P), Stmt(Stmt) {

}

/**
 * Parse a statement assignment Expression
 * @param Block
 * @param VarRef
 * @param Success true on Success or false on Error
 * @return the ASTExpr
 */
ASTExpr *ExprParser::ParseAssignExpr(ASTVarRef *VarRef) {

    // Parsing =
    if (P->Tok.is(tok::equal)) {
        P->ConsumeToken();
        return ParseExpr();
    }

    // Create First Expr
    ASTVarRefExpr *First = P->Builder.CreateExpr(Stmt, VarRef);
    Group.push_back(First);

    // Parse binary assignment operator
    BinaryOpKind Op;
    switch (P->Tok.getKind()) {

        // Arithmetic
        case tok::plusequal:
            Op = BinaryOpKind::ARITH_ADD;
            break;
        case tok::minusequal:
            Op = BinaryOpKind::ARITH_SUB;
            break;
        case tok::starequal:
            Op = BinaryOpKind::ARITH_MUL;
            break;
        case tok::slashequal:
            Op = BinaryOpKind::ARITH_DIV;
            break;
        case tok::percentequal:
            Op = BinaryOpKind::ARITH_MOD;
            break;

            // Bit
        case tok::ampequal:
            Op = BinaryOpKind::ARITH_AND;
            break;
        case tok::pipeequal:
            Op = BinaryOpKind::ARITH_OR;
            break;
        case tok::caretequal:
            Op = BinaryOpKind::ARITH_XOR;
            break;
        case tok::lesslessequal:
            Op = BinaryOpKind::ARITH_SHIFT_L;
            break;
        case tok::greatergreaterequal:
            Op = BinaryOpKind::ARITH_SHIFT_R;
            break;
        default:
            assert(0 && "Accept Only assignment operators");
    }
    RawBinaryOperator *RawOp = new RawBinaryOperator(P->ConsumeToken(), Op);
    Group.push_back(RawOp);

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
    FLY_DEBUG_MESSAGE("Parser", "ParseExpr", "IsFirst=" + std::to_string(IsFirst));

    // The parsed ASTExpr
    ASTExpr *Expr = nullptr;

    // Location of the starting expression
    if (P->Tok.is(tok::l_paren)) { // Start a new Group of Expressions
        P->ConsumeParen();

        ExprParser SubP(P, Stmt);
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
        ASTValue *Val = P->ParseValue();
        if (Val) // Parse a value
            Expr = P->Builder.CreateExpr(Stmt,Val);
    } else if (P->isIdentifier()) { // Ex. a or a++ or func()
        SourceLocation Loc;
        llvm::StringRef Name;
        llvm::StringRef NameSpace;
        if (P->ParseIdentifier(Loc, Name, NameSpace)) {
            Expr = ParseExpr(Loc, Name, NameSpace);
        }
    } else if (P->isUnaryPreOperator(P->Tok)) { // Ex. ++a or --a or !a
        Expr = ParseUnaryPreExpr(P); // Parse Unary Post Expression
    } else {
        return P->Builder.CreateExpr(Stmt); // return an ASTEmptyExpr
    }

     // Error: missing expression
     if (Expr == nullptr) {
         P->Diag(P->Tok.getLocation(), diag::err_parser_miss_expr);
         return nullptr;
     }

    // Check if binary operator exists
    if (P->isBinaryOperator()) { // Parse Binary Expression
        Group.push_back(Expr);
        BinaryOpKind Op = ParseBinaryOperator();
        RawBinaryOperator *RawOp = new RawBinaryOperator(P->ConsumeToken(), Op);
        Group.push_back(RawOp);
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
        const SourceLocation &QuestionLoc = P->ConsumeToken();

        ExprParser SubSecond(P, Stmt);
        ASTExpr *True = SubSecond.ParseExpr();

        if (P->Tok.getKind() == tok::colon) {
            const SourceLocation &ColonLoc = P->ConsumeToken();

            ExprParser SubThird(P, Stmt);
            ASTExpr *False = SubThird.ParseExpr();
            if (False != nullptr)
                return P->Builder.CreateTernaryExpr(Stmt, Expr, QuestionLoc, True, ColonLoc, False);
        }

        // Error: Invalid operator in Ternary condition
        P->Diag(P->Tok.getLocation(), diag::err_parser_miss_oper);
        return nullptr;
    }

    return Expr;
}

ASTExpr *ExprParser::ParseExpr(SourceLocation &Loc, llvm::StringRef Name, llvm::StringRef NameSpace) {
    if (P->Tok.is(tok::l_paren)) { // Ex. a()
        ASTFunctionCall *Call = P->ParseFunctionCall(Stmt, Loc, Name, NameSpace);
        if (Call) {
            return P->Builder.CreateExpr(Stmt, Call);
        }
        //TODO add Error on Call parsing
    } else { // parse variable post increment/decrement or simple var
        ASTVarRef *VarRef = P->Builder.CreateVarRef(Loc, Name.str(), NameSpace.str());
        if (P->isUnaryPostOperator()) { // Ex. a++ or a--
            return ParseUnaryPostExpr(VarRef); // Parse Unary Pre Expression
        } else {
            // Simple Var
            return P->Builder.CreateExpr(Stmt, VarRef);
        }
    }
    return nullptr;
}

/**
 * Parse unary pre operators ++a, --a, !a
 * @param Block
 * @param VarRef
 * @param Success
 * @return
 */
ASTUnaryGroupExpr* ExprParser::ParseUnaryPostExpr(ASTVarRef *VarRef) {
    ASTVarRefExpr *VarRefExpr = P->Builder.CreateExpr(Stmt, VarRef);
        UnaryOpKind Op;
        switch (P->Tok.getKind()) {
            case tok::exclaim:
                Op = UnaryOpKind::LOGIC_NOT;
                break;
            case tok::plusplus:
                Op = UnaryOpKind::ARITH_INCR;
                break;
            case tok::minusminus:
                Op = UnaryOpKind::ARITH_DECR;
                break;
            default:
                assert(0 && "Unary Operator not accepted");
        }
        P->ConsumeToken();
        return P->Builder.CreateUnaryExpr(Stmt, VarRef->getLocation(), Op, UnaryOptionKind::UNARY_POST, VarRefExpr);
}

/**
 * Parse unary operators ++a, --a, !a
 * @param Block
 * @param Success
 * @return
 */
ASTUnaryGroupExpr* ExprParser::ParseUnaryPreExpr(Parser *P) {

    UnaryOpKind Op;
    switch (P->Tok.getKind()) {
        case tok::exclaim:
            Op = UnaryOpKind::LOGIC_NOT;
            break;
        case tok::plusplus:
            Op = UnaryOpKind::ARITH_INCR;
            break;
        case tok::minusminus:
            Op = UnaryOpKind::ARITH_DECR;
            break;
        default:
            assert(0 && "Unary Pre Operator not accepted");
    }
    P->ConsumeToken();

    // Check var identifier
    if (P->Tok.isAnyIdentifier()) {

        llvm::StringRef Name;
        llvm::StringRef NameSpace;
        SourceLocation Loc;

        if (P->ParseIdentifier(Loc, Name, NameSpace)) {
            ASTVarRef *VarRef = P->Builder.CreateVarRef(Loc, Name.str(), NameSpace.str());
            ASTVarRefExpr *VarRefExpr = P->Builder.CreateExpr(Stmt, VarRef);
            return P->Builder.CreateUnaryExpr(Stmt, Loc, Op, UnaryOptionKind::UNARY_PRE, VarRefExpr);
        }
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
BinaryOpKind ExprParser::ParseBinaryOperator() {
    FLY_DEBUG("Parser", "ParseBinaryOperator");

    BinaryOpKind Op;
    switch (P->Tok.getKind()) {

        // Binary Arithmetic
        case tok::plus:
            return BinaryOpKind::ARITH_ADD;
        case tok::minus:
            return BinaryOpKind::ARITH_SUB;
        case tok::star:
            return BinaryOpKind::ARITH_MUL;
        case tok::slash:
            return BinaryOpKind::ARITH_DIV;
        case tok::percent:
            return BinaryOpKind::ARITH_MOD;
        case tok::amp:
            return BinaryOpKind::ARITH_AND;
        case tok::pipe:
            return BinaryOpKind::ARITH_OR;
        case tok::caret:
            return BinaryOpKind::ARITH_XOR;
        case tok::lessless:
            return BinaryOpKind::ARITH_SHIFT_L;
        case tok::greatergreater:
            return BinaryOpKind::ARITH_SHIFT_R;

            // Logic
        case tok::ampamp:
            return BinaryOpKind::LOGIC_AND;
        case tok::pipepipe:
            return BinaryOpKind::LOGIC_OR;

            // Comparator
        case tok::less:
            return BinaryOpKind::COMP_LT;
        case tok::lessequal:
            return BinaryOpKind::COMP_LTE;
        case tok::greater:
            return BinaryOpKind::COMP_GT;
        case tok::greaterequal:
            return BinaryOpKind::COMP_GTE;
        case tok::exclaimequal:
            return BinaryOpKind::COMP_NE;
        case tok::equalequal:
            return BinaryOpKind::COMP_EQ;

    }
    assert(0 && "Binary Operator not accepted");
}

void ExprParser::UpdateBinaryGroup(bool NoPrecedence) {
    std::vector<ASTExpr *> Result;
    RawBinaryOperator *Op;
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
            Op = (RawBinaryOperator *) *It;

            // The second operator
            if (++It != Group.end()) {
                Second = *It;
                if (NoPrecedence || Op->isPrecedence()) {
                    First = Result.back();
                    Result.pop_back();
                    Result.push_back(P->Builder.CreateBinaryExpr(Stmt, Op->getLocation(), Op->getOp(), First, Second));
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
