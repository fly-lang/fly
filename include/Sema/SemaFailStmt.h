//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaFailStmt.h - Sema Fail Statement header
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_FAILSTMT_H
#define FLY_SEMA_FAILSTMT_H

#include "Sema/SemaStmt.h"

namespace fly {

    class SemaExpr;

    class SemaFailStmt : public SemaStmt {

        SemaExpr *First  = nullptr;  // main error code/message

        SemaExpr *Second = nullptr;  // optional secondary info

        SemaExpr *Third  = nullptr;  // optional tertiary info

    public:

        explicit SemaFailStmt(ASTStmt *AST);

        ~SemaFailStmt() override = default;

        SemaExpr *getFirst()  const;
        SemaExpr *getSecond() const;
        SemaExpr *getThird()  const;

        void setFirst(SemaExpr *E);
        void setSecond(SemaExpr *E);
        void setThird(SemaExpr *E);

        void accept(SemaVisitor &Visitor) override;
    };
}

#endif //FLY_SEMA_FAILSTMT_H

