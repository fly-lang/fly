//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaLoopInStmt.h - Sema Loop-In Statement header
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_LOOPINSTMT_H
#define FLY_SEMA_LOOPINSTMT_H

#include "Sema/SemaStmt.h"

namespace fly {

    class SemaExpr;

    class SemaLoopInStmt : public SemaStmt {

        SemaExpr *Item;   // loop variable

        SemaExpr *List;   // iterable expression

        SemaStmt *Body;

    public:

        SemaLoopInStmt(ASTStmt *AST, SemaExpr *Item, SemaExpr *List, SemaStmt *Body);

        ~SemaLoopInStmt() override = default;

        SemaExpr *getItem() const;

        SemaExpr *getList() const;

        SemaStmt *getBody() const;

        void accept(SemaVisitor &Visitor) override;
    };
}

#endif //FLY_SEMA_LOOPINSTMT_H

