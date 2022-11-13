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

namespace fly {

    /**
     * Raw Binary Operator
     */
    class RawBinaryOperator : public ASTEmptyExpr {

        ASTBinaryOperatorKind Op;
        bool Precedence;

    public:
        RawBinaryOperator(const SourceLocation &Loc, ASTBinaryOperatorKind Op) : ASTEmptyExpr(Loc), Op(Op) {
            Precedence = Op == ASTBinaryOperatorKind::ARITH_MUL ||
                         Op == ASTBinaryOperatorKind::ARITH_DIV ||
                         Op == ASTBinaryOperatorKind::ARITH_MOD;
        }

        ASTBinaryOperatorKind getOp() const {
            return Op;
        }

        bool isPrecedence() const {
            return Precedence;
        }

        std::string str() const override {
            return std::to_string((int) Op);
        }
    };

    class ExprParser {

        friend class ASTExpr;

        Parser *P;

        ASTStmt *Stmt;

        std::vector<ASTExpr *> Group;

    public:
        ExprParser(Parser *P, ASTStmt *Stmt);

        ASTExpr *ParseAssignExpr(ASTVarRef *VarRef = nullptr);

        ASTExpr *ParseExpr(bool IsFirst = true);

        ASTExpr *ParseExpr(ASTIdentifier *Identifier);

        ASTUnaryGroupExpr *ParseUnaryPreExpr(Parser *P);

    private:
        ASTUnaryGroupExpr *ParseUnaryPostExpr(ASTVarRef *VarRef);

        ASTBinaryOperatorKind ParseBinaryOperator();

        void UpdateBinaryGroup(bool NoPrecedence);
    };

}

#endif //FLY_EXPRPARSER_H
