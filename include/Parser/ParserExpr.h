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
        explicit ParserExpr(Parser *P);

        static ASTExpr *Parse(Parser *P);

    private:
        ASTExpr *ParsePrimary();

        ASTUnaryOpExpr *ParseUnaryExpr();

        ASTBinaryOpExpr *ParseBinaryExpr(ASTExpr *LeftExpr, Token OpToken, Precedence Precedence);

        ASTTernaryOpExpr *ParseTernaryExpr(ASTExpr *ConditionExpr);

        ASTExpr *ParseNewExpr(Parser *P);

    };

}

#endif //FLY_PARSEREXPR_H
