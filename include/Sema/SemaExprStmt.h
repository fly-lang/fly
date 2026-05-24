//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaExprStmt.h - expression statement semantic analysis
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_EXPRSTMT_H
#define FLY_SEMA_EXPRSTMT_H

#include "Sema/SemaStmt.h"

namespace fly {

    class SemaExpr;

    class SemaExprStmt : public SemaStmt {

        SemaExpr *Expr;

    public:

        SemaExprStmt(ASTStmt *AST, SemaExpr *Expr);

        ~SemaExprStmt() override = default;

        SemaExpr *getExpr() const;

        std::string str() const override;

        void accept(SemaVisitor &Visitor) override;
    };
}

#endif //FLY_SEMA_EXPRSTMT_H

