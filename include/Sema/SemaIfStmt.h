//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaIfStmt.h - if statement semantic analysis
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_IFSTMT_H
#define FLY_SEMA_IFSTMT_H

#include "Sema/SemaStmt.h"
#include "llvm/ADT/SmallVector.h"
#include <utility>

namespace fly {

    class SemaExpr;

    // Represents a single condition+body clause (used for elsif/case)
    struct SemaRuleStmt {
        SemaExpr *Expr;   // condition or case value
        SemaStmt *Stmt;   // body
    };

    class SemaIfStmt : public SemaStmt {

        SemaExpr *Cond;

        SemaStmt *Then;

        llvm::SmallVector<SemaRuleStmt, 4> Elsif;

        SemaStmt *Else = nullptr;

    public:

        SemaIfStmt(ASTStmt *AST, SemaExpr *Cond, SemaStmt *Then);

        ~SemaIfStmt() override = default;

        SemaExpr *getCond() const;

        SemaStmt *getThen() const;

        void addElsif(SemaExpr *Expr, SemaStmt *Stmt);

        const llvm::SmallVector<SemaRuleStmt, 4> &getElsif() const;

        SemaStmt *getElse() const;

        void setElse(SemaStmt *Else);

        std::string str() const override;

        void accept(SemaVisitor &Visitor) override;
    };
}

#endif //FLY_SEMA_IFSTMT_H

