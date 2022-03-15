//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/ExprParser.cpp - Expression Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include <AST/ASTResolver.h>
#include "Parser/ExprParser.h"
#include "Basic/Debug.h"

using namespace fly;

ExprParser::ExprParser(Parser *P) : P(P){

}

/**
 * Parse a statement assignment Expression
 * @param Block
 * @param VarRef
 * @param Success true on Success or false on Error
 * @return the ASTExpr
 */
ASTExpr *ExprParser::ParseAssignmentExpr(ASTBlock *Block, ASTVarRef *VarRef) {
    // Parsing =
    if (P->Tok.is(tok::equal)) {
        P->ConsumeToken();
        return ParseExpr(Block);
    }

    // Parsing =, +=, -=, ...
    if (isAssignOperator(P->Tok)) {

        // Create First Expr
        ASTVarRefExpr *Expr = new ASTVarRefExpr(VarRef);
        Group.push_back(Expr);

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
        return ParseExpr(Block);
    }

    // Statement without assignment
    // int a
    // Type a
    return nullptr;
}

/**
 * Parse all Expressions
 * @param Block
 * @param Success true on Success or false on Error
 * @param ParentGroup
 * @return the ASTExpr
 */
ASTExpr *ExprParser::ParseExpr(ASTBlock *Block, bool Start) {
    assert(Block != nullptr && "Block is nullptr");
    FLY_DEBUG("Parser", "ParseExpr");

    // Return
    ASTExpr *Expr = nullptr;

    // Location of the starting expression
    if (P->Tok.is(tok::l_paren)) { // Start a new Group of Expressions
        P->ConsumeParen();

        ExprParser SubP(P);
        Expr = SubP.ParseExpr(Block);
        if (Expr != nullptr) {
            if (P->Tok.is(tok::r_paren)) {
                P->ConsumeParen();
                return Expr; // Ok
            }

            // Error: parenthesis unclosed
            P->Diag(P->Tok.getLocation(), diag::err_paren_unclosed);
            return nullptr;
        }
        return nullptr;
    }

    // Populate Expr

    // Ex. 1
    if (P->isValue()) {
        ASTValue *Val = P->ParseValue();
        if (Val != nullptr) // Parse a value
            Expr = new ASTValueExpr(Val);
    } else if (P->Tok.isAnyIdentifier()) { // Ex. a or a++ or func()
        llvm::StringRef Name;
        llvm::StringRef NameSpace;
        SourceLocation IdLoc;
        if (P->ParseIdentifier(Name, NameSpace, IdLoc)) {
            Expr = ParseExpr(Block, Name, NameSpace, IdLoc);
        }
        // TODO add Error of ParseIdentifier()
    } else if (isUnaryPreOperator()) { // Ex. ++a or --a or !a
        Expr = ParseUnaryPreExpr(Block); // Parse Unary Post Expression
    } else {
        // Error: unexpected operator
        P->Diag(P->Tok.getLocation(), diag::err_parser_unexpect_oper);
        return nullptr;
    }

    // Error: missing expression
    if (Expr == nullptr) {
        P->Diag(P->Tok.getLocation(), diag::err_parser_miss_expr);
        return nullptr;
    }

    // Check if binary operator exists
    if (isBinaryOperator()) { // Parse Binary Expression
        Group.push_back(Expr);
        BinaryOpKind Op = ParseBinaryOperator();
        RawBinaryOperator *RawOp = new RawBinaryOperator(P->ConsumeToken(), Op);
        Group.push_back(RawOp);
        Expr = ParseExpr(Block, false);

        // Error: missing second operator
        if (Expr == nullptr) {
            P->Diag(P->Tok.getLocation(), diag::err_parser_miss_oper);
            return nullptr;
        }

        if (Start) {
            // This is the last item in the expression
            Group.push_back(Expr);

            // Update Group
            UpdateBinaryGroup(false);
            UpdateBinaryGroup(true);
            assert(Group.size() == 1 && "Only one Group entry at the end");
            assert(Group[0]->getKind() == EXPR_GROUP && "Only one Group entry at the end");
            assert(((ASTGroupExpr *) Group[0])->getGroupKind() == GROUP_BINARY && "Only one Group entry at the end");
            return (ASTBinaryGroupExpr *) Group[0];
        }

    } else if (isTernaryOperator()) { // Parse Ternary Expression
        return ParseTernaryExpr(Block, Expr);
    }

    return Expr;
}

ASTExpr *ExprParser::ParseExpr(ASTBlock *Block, llvm::StringRef Name, llvm::StringRef NameSpace, SourceLocation IdLoc) {
    if (P->Tok.is(tok::l_paren)) { // Ex. a()
        ASTFuncCall *Call = P->ParseFunctionCall(Block, Name, NameSpace, IdLoc);
        if (Call != nullptr && Block->Top->getNode()->AddUnrefCall(Call)) { // To Resolve on the next
            return new ASTFuncCallExpr(Call);
        }
        // TODO add Error of Call or AddUnrefCall()
    } else { // parse variable post increment/decrement or simple var
        ASTVarRef *VarRef = new ASTVarRef(IdLoc, Name.str(), NameSpace.str());
        if (ASTResolver::ResolveVarRef(Block, VarRef)) {
            if (isUnaryPostOperator()) { // Ex. a++ or a--
                return ParseUnaryPostExpr(Block, VarRef); // Parse Unary Pre Expression
            } else {
                // Simple Var
                return new ASTVarRefExpr(VarRef);
            }
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
ASTUnaryGroupExpr* ExprParser::ParseUnaryPostExpr(ASTBlock *Block, ASTVarRef *VarRef) {
    ASTVarRefExpr *VarRefExpr = new ASTVarRefExpr(VarRef);
    if (ASTResolver::ResolveVarRef(Block, VarRef)) {

        UnaryOpKind Op;
        switch (P->Tok.getKind()) {
            case tok::exclaim:
                Op = LOGIC_NOT;
                break;
            case tok::plusplus:
                Op = ARITH_INCR;
                break;
            case tok::minusminus:
                Op = ARITH_DECR;
                break;
            default:
                assert(0 && "Unary Operator not accepted");
        }
        P->ConsumeToken();
        return new ASTUnaryGroupExpr(VarRef->getLocation(), Op, UNARY_POST, VarRefExpr);
    }
    return nullptr;
}

/**
 * Parse unary operators ++a, --a, !a
 * @param Block
 * @param Success
 * @return
 */
ASTUnaryGroupExpr* ExprParser::ParseUnaryPreExpr(ASTBlock *Block) {

    UnaryOpKind Op;
    switch (P->Tok.getKind()) {
        case tok::exclaim:
            Op = LOGIC_NOT;
            break;
        case tok::plusplus:
            Op = ARITH_INCR;
            break;
        case tok::minusminus:
            Op = ARITH_DECR;
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

        if (P->ParseIdentifier(Name, NameSpace, Loc)) {
            ASTVarRef *VarRef = new ASTVarRef(Loc, Name.str(), NameSpace.str());
            ASTVarRefExpr *VarRefExpr = new ASTVarRefExpr(VarRef);
            if (ASTResolver::ResolveVarRef(Block, VarRef)) {
                return new ASTUnaryGroupExpr(Loc, Op, UNARY_PRE, VarRefExpr);
            }
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
                    Result.push_back(new ASTBinaryGroupExpr(First->getLocation(), Op->getOp(), First, Second));
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

ASTTernaryGroupExpr* ExprParser::ParseTernaryExpr(ASTBlock *Block, ASTExpr *First) {

    // Parse if condition after the ?
    if (P->Tok.getKind() == tok::question) {
        P->ConsumeToken();

        // Parse else condition after the :
        ASTExpr *Second = ParseExpr(Block);
        if (Second) {
            if (P->Tok.getKind() == tok::semi) {
                P->ConsumeToken();

                ASTExpr *Third = ParseExpr(Block);
                if (Third != nullptr)
                    return new ASTTernaryGroupExpr(First->getLocation(), First, Second, Third);
            }
        }

        // Error: missing operator
        P->Diag(P->Tok.getLocation(), diag::err_parser_miss_oper);
        return nullptr;
    }
}

/**
 * Check if Token is one of the Assign Operators
 * @return true on Success or false on Error
 */
bool ExprParser::isAssignOperator(Token &Tok) {
    return Tok.isOneOf(tok::equal, tok::plusequal, tok::minusequal, tok::starequal, tok::slashequal,
                          tok::percentequal, tok::ampequal, tok::pipeequal, tok::caretequal, tok::lesslessequal,
                          tok::greatergreaterequal);
}

/**
 * Check if Token is one of the Unary Pre Operators
 * @return true on Success or false on Error
 */
bool ExprParser::isUnaryPreOperator() {
    return P->Tok.isOneOf(tok::plusplus, tok::minusminus, tok::exclaim);
}

/**
 * Check if Token is one of the Unary Post Operators
 * @return true on Success or false on Error
 */
bool ExprParser::isUnaryPostOperator() {
    return P->Tok.isOneOf(tok::plusplus, tok::minusminus);
}

/**
 * Check if Token is one of Binary Operators
 * @return true on Success or false on Error
 */
bool ExprParser::isBinaryOperator() {
    return P->Tok.isOneOf(
            // Arithmetci Operators
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
            tok::greatergreater // >> shift right
    );
}

/**
 * Check if Token is one of Binary Operators
 * @return true on Success or false on Error
 */
bool ExprParser::isTernaryOperator() {
    return P->Tok.isOneOf(tok::question, tok::colon);
}
