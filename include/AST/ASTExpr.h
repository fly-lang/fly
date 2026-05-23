//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTExpr.h - AST expression base header
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_EXPR_H
#define FLY_AST_EXPR_H

#include "ASTNode.h"

namespace fly {


    enum class ASTExprKind : char {
        EXPR_VALUE,
        EXPR_IDENTIFIER,
        EXPR_MEMBER,
        EXPR_CALL,
        EXPR_UNARY,
        EXPR_BINARY,
        EXPR_TERNARY,
        EXPR_CAST
    };

    /**
     * Expression Abstract Class.
     * Name-resolved nodes (ASTIdentifier, ASTCall, ASTMember) store a Symbol*.
     * The resolved Sema expression is maintained in the Sema tree, not on the AST.
     */
    class ASTExpr : public ASTNode {

        friend class ASTBuilder;

    protected:

        const ASTExprKind ExprKind;

        ASTExpr *Parent = nullptr;

        ASTExpr *Child  = nullptr;

        ASTExpr(const SourceLocation &Loc, ASTExprKind Kind, ASTExpr *Parent = nullptr, ASTExpr *Child = nullptr);

    public:

        ASTExprKind getExprKind() const;

        ASTExpr *getParent() const;

        ASTExpr *getChild() const;

        void setParent(ASTExpr *Parent);

        void setChild(ASTExpr *Child);


        std::string str() const override;
    };
}

#endif //FLY_AST_EXPR_H
