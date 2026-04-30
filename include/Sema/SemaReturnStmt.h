//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaReturnStmt.h - Sema Return Statement header
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_RETURNSTMT_H
#define FLY_SEMA_RETURNSTMT_H

#include "Sema/SemaStmt.h"

namespace fly {

    class SemaExpr;

    class SemaReturnStmt : public SemaStmt {

        SemaExpr *Expr = nullptr;

    public:

        explicit SemaReturnStmt(ASTStmt *AST, SemaExpr *Expr = nullptr);

        ~SemaReturnStmt() override = default;

        SemaExpr *getExpr() const;

        void accept(SemaVisitor &Visitor) override;
    };
}

#endif //FLY_SEMA_RETURNSTMT_H

