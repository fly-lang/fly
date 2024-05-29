//===--------------------------------------------------------------------------------------------------------------===//
// include/Parser/ExprParser.h - Expression Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_EXPRPARSER_H
#define FLY_EXPRPARSER_H

#include "Parser.h"
#include "AST/ASTExpr.h"
#include "AST/ASTGroupExpr.h"

namespace fly {

    class ExprParser {

        friend class ASTExpr;

        Parser *P;

        std::vector<ASTExpr *> Group;

    public:
        explicit ExprParser(Parser *P);

        ASTExpr *ParseAssignExpr(ASTVarRef *VarRef = nullptr);

        ASTExpr *ParseExpr(bool IsFirst = true);

        ASTExpr *ParseExpr(ASTIdentifier *Identifier);

        ASTExpr *ParseNewExpr(Parser *P);

        ASTUnaryGroupExpr *ParseUnaryPreExpr(Parser *P);

    private:
        ASTUnaryGroupExpr *ParseUnaryPostExpr(ASTVarRef *VarRef);

        ASTBinaryOperatorKind ParseBinaryOperator();

        void UpdateBinaryGroup(bool NoPrecedence);
    };

}

#endif //FLY_EXPRPARSER_H
