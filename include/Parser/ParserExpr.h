//===--------------------------------------------------------------------------------------------------------------===//
// include/Parser/ExprParser.h - Expression Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_PARSEREXPR_H
#define FLY_PARSEREXPR_H

#include "Parser.h"
#include "AST/ASTExpr.h"
#include "AST/ASTOpExpr.h"

namespace fly {

    class ParserExpr {

        friend class ASTExpr;

        Parser *P;

        ASTExpr *Expr = nullptr;

    public:
        ParserExpr(Parser *P, ASTExpr *Expr = nullptr);

        static ASTExpr *Parse(Parser *P, ASTExpr *Expr = nullptr);

    private:
        ASTExpr *ParsePrimary(bool Expected = false);

        ASTBinaryOpExpr *ParseBinaryExpr(ASTExpr *LeftExpr, Token OpToken, Precedence Precedence);

        ASTTernaryOpExpr *ParseTernaryExpr(ASTExpr *ConditionExpr);

        ASTExpr *ParseNewExpr();

        bool isNewOperator(Token &Tok);

        bool isUnaryPreOperator(Token &Tok);

        bool isUnaryPostOperator();

        bool isBinaryOperator();

        bool isTernaryOperator();

    };

}

#endif //FLY_PARSEREXPR_H
