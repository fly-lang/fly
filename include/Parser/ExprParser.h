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
    class RawBinaryOperator : public ASTEmptyExpr {

        BinaryOpKind Op;
        bool Precedence;

    public:
        RawBinaryOperator(const SourceLocation &Loc, BinaryOpKind Op) : ASTEmptyExpr(Loc), Op(Op) {
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

        std::string str() const override {
            return std::to_string(Op);
        }
    };

    class ExprParser {

        Parser *P;

        ASTStmt *Stmt;

        std::vector<ASTExpr *> Group;

    public:
        ExprParser(Parser *P, ASTStmt *Stmt);

        ASTExpr *ParseAssignExpr(ASTVarRef *VarRef = nullptr);

        ASTExpr *ParseExpr(bool IsFirst = true);

        ASTExpr *ParseExpr(SourceLocation &Loc, llvm::StringRef Name, llvm::StringRef NameSpace);

        ASTUnaryGroupExpr *ParseUnaryPreExpr(Parser *P);

    private:
        ASTUnaryGroupExpr *ParseUnaryPostExpr(ASTVarRef *VarRef);

        BinaryOpKind ParseBinaryOperator();

        void UpdateBinaryGroup(bool NoPrecedence);
    };

}

#endif //FLY_EXPRPARSER_H
