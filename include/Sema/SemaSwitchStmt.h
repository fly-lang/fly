//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaSwitchStmt.h - Sema Switch Statement header
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_SWITCHSTMT_H
#define FLY_SEMA_SWITCHSTMT_H

#include "Sema/SemaStmt.h"
#include "Sema/SemaIfStmt.h"  // for SemaRuleStmt
#include "llvm/ADT/SmallVector.h"

namespace fly {

    class SemaExpr;

    class SemaSwitchStmt : public SemaStmt {

        SemaExpr *Expr;

        llvm::SmallVector<SemaRuleStmt, 8> Cases;

        SemaStmt *Default = nullptr;

    public:

        SemaSwitchStmt(ASTStmt *AST, SemaExpr *Expr);

        ~SemaSwitchStmt() override = default;

        SemaExpr *getExpr() const;

        void addCase(SemaExpr *CaseExpr, SemaStmt *CaseStmt);

        const llvm::SmallVector<SemaRuleStmt, 8> &getCases() const;

        SemaStmt *getDefault() const;

        void setDefault(SemaStmt *Default);

        void accept(SemaVisitor &Visitor) override;
    };
}

#endif //FLY_SEMA_SWITCHSTMT_H

