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

namespace fly {

    /**
     * Raw Binary Operator
     */
    class RawBinaryOperator : public ASTExpr {

        BinaryOpKind Op;
        bool Precedence;

    public:
        RawBinaryOperator(const SourceLocation &Loc, BinaryOpKind Op) : ASTExpr(Loc), Op(Op) {
            Precedence = Op == BinaryOpKind::ARITH_MUL ||
                         Op == BinaryOpKind::ARITH_DIV ||
                         Op == BinaryOpKind::ARITH_MOD;
        }

        BinaryOpKind getOp() const {
            return Op;
        }

        bool isPrecedence() const {
            return Precedence;
        }

        ASTType *getType() const override {
            return nullptr;
        }

        ASTExprKind getExprKind() const override {
            return ASTExprKind::EXPR_GROUP;
        }

        std::string str() const override {
            return std::string();
        }
    };

    class ExprParser {

        Parser *P;
        std::vector<ASTExpr *> Group;

    public:
        ExprParser(Parser *P);

        ASTExpr *ParseAssignmentExpr(ASTVarRef *VarRef);
        ASTExpr *ParseExpr(bool Start = true);
        ASTExpr *ParseExpr(llvm::StringRef Name, llvm::StringRef NameSpace, SourceLocation IdLoc);
        static bool isAssignOperator(Token &Tok);
        static bool isUnaryPreOperator(Token &Tok);
        static ASTUnaryGroupExpr *ParseUnaryPreExpr(Parser *P);

    private:
        ASTUnaryGroupExpr *ParseUnaryPostExpr(ASTVarRef *VarRef);

        BinaryOpKind ParseBinaryOperator();
        void UpdateBinaryGroup(bool NoPrecedence);
        ASTTernaryGroupExpr * ParseTernaryGroup(ASTExpr *First);
        bool isUnaryPostOperator();
        bool isBinaryOperator();
        bool isTernaryOperator();
    };

}

#endif //FLY_EXPRPARSER_H
