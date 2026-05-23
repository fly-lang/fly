//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTBinary.h - AST binary expression header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_BINARY_H
#define FLY_AST_BINARY_H

#include "ASTExpr.h"

namespace fly {

    enum class ASTBinaryKind {

        // Arithmetic
        OP_BINARY_ARITH_ADD,
        OP_BINARY_ARITH_SUB,
        OP_BINARY_ARITH_MUL,
        OP_BINARY_ARITH_DIV,
        OP_BINARY_ARITH_MOD,
        OP_BINARY_ARITH_AND,
        OP_BINARY_ARITH_OR ,
        OP_BINARY_ARITH_XOR,
        OP_BINARY_ARITH_SHIFT_L,
        OP_BINARY_ARITH_SHIFT_R,

        // Logic
        OP_BINARY_LOGIC_AND,
        OP_BINARY_LOGIC_OR,

        // Comparison
        OP_BINARY_COMPARE_EQ ,
        OP_BINARY_COMPARE_NE ,
        OP_BINARY_COMPARE_GT ,
        OP_BINARY_COMPARE_GTE,
        OP_BINARY_COMPARE_LT ,
        OP_BINARY_COMPARE_LTE,

        // ASSIGN
        OP_BINARY_ASSIGN,
        OP_BINARY_ASSIGN_AND,
        OP_BINARY_ASSIGN_MUL,
        OP_BINARY_ASSIGN_ADD,
        OP_BINARY_ASSIGN_SUB,
        OP_BINARY_ASSIGN_DIV,
        OP_BINARY_ASSIGN_MOD,
        OP_BINARY_ASSIGN_SHIFT_L,
        OP_BINARY_ASSIGN_SHIFT_R,
        OP_BINARY_ASSIGN_XOR,
        OP_BINARY_ASSIGN_OR
    };

    /**
     * Binary Operator Expression
     */
    class ASTBinary : public ASTExpr {

        friend class ASTBuilder;

        ASTBinaryKind BinaryKind;

        SourceLocation OpLocation;

        ASTExpr *LeftExpr = nullptr;

        ASTExpr *RightExpr = nullptr;

        ASTBinary(ASTBinaryKind OpKind, const SourceLocation &OpLocation,
                        ASTExpr *LeftExpr, ASTExpr *RightExpr);

    public:

        void accept(ASTVisitor& Visitor) override;

        bool isArith() const;

        bool isCompare() const;

        bool isAssign() const;

        bool isLogic() const;

        ASTBinaryKind getBinaryKind() const;

        SourceLocation &getOpLocation();

        ASTExpr *getLeftExpr() const;

        ASTExpr *getRightExpr() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_BINARY_H

