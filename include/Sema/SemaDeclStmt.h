//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaDeclStmt.h - declaration statement semantic analysis
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_DECLSTMT_H
#define FLY_SEMA_DECLSTMT_H

#include "Sema/SemaStmt.h"

namespace fly {

    class SemaLocalVar;
    class SemaExpr;

    class SemaDeclStmt : public SemaStmt {

        SemaLocalVar *Var;

        SemaExpr *Expr;  // nullable — init expression

    public:

        SemaDeclStmt(ASTStmt *AST, SemaLocalVar *Var, SemaExpr *Expr = nullptr);

        ~SemaDeclStmt() override = default;

        SemaLocalVar *getVar() const;

        SemaExpr *getExpr() const;

        std::string str() const override;

        void accept(SemaVisitor &Visitor) override;
    };
}

#endif //FLY_SEMA_DECLSTMT_H

