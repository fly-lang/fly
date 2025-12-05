//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTAssignStmt.h - AST Variable statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_ASSIGNSTMT
#define FLY_AST_ASSIGNSTMT

#include "ASTStmt.h"

namespace fly {

    class ASTIdentifier;

    enum class ASTAssignOperatorKind {
        EQUAL,
        PLUS_EQUAL,
        MINUS_EQUAL,
        AMP_EQUAL,
        PIPE_EQUAL,
        STAR_EQUAL,
        SLASH_EQUAL,
        PERCENT_EQUAL,
        LESSLESS_EQUAL,
        GREATERGREATER_EQUAL,
        CARET_EQUAL
    };

    /**
     * Assign somethings to a Local Var
     * Ex.
     *  a = 1
     */
    class ASTAssignStmt : public ASTStmt {

        friend class ASTBuilder;
        friend class ASTBuilderStmt;

        ASTExpr *Source;

        ASTAssignOperatorKind Kind;

        ASTExpr *Target = nullptr;

        ASTAssignStmt(const SourceLocation &Loc, ASTExpr *Source, ASTAssignOperatorKind AssignOperatorKind);

    public:

        void accept(ASTVisitor& Visitor) override;

        ASTExpr *getSource() const;

        ASTAssignOperatorKind getOpKind() const;

        ASTExpr *getTarget() const;

        void setExpr(ASTExpr *);

        std::string str() const override;
    };
}

#endif //FLY_AST_ASSIGNSTMT
