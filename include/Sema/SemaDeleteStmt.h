//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaDeleteStmt.h - delete statement semantic analysis
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_DELETESTMT_H
#define FLY_SEMA_DELETESTMT_H

#include "Sema/SemaStmt.h"

namespace fly {

    class SemaExpr;

    class SemaDeleteStmt : public SemaStmt {

        SemaExpr *Expr;

    public:

        SemaDeleteStmt(ASTStmt *AST, SemaExpr *Expr);

        ~SemaDeleteStmt() override = default;

        SemaExpr *getExpr() const;

        void accept(SemaVisitor &Visitor) override;
    };
}

#endif //FLY_SEMA_DELETESTMT_H

