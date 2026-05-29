//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaReturnStmt.h - return statement semantic analysis
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

        std::string str() const override;

        void accept(SemaVisitor &Visitor) override;
    };
}

#endif //FLY_SEMA_RETURNSTMT_H

