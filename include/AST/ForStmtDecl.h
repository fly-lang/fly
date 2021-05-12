//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ForStmtDecl.h - For Statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_FORSTMTDECL_H
#define FLY_FORSTMTDECL_H

#include "StmtDecl.h"

namespace fly {

    class ForStmtDecl : public StmtDecl {

        enum StmtKind StmtKind = StmtKind::D_STMT_FOR;

        std::vector<VarDecl *> Init;
        CondExpr *Condition;
        std::vector<IncDecExpr *> Count;

    public:
        ForStmtDecl(const SourceLocation &Loc, StmtDecl *Parent);

        const std::vector<VarDecl *> &getInit() const;

        const CondExpr *getCondition() const;

        const std::vector<IncDecExpr *> &getCount() const;

    };
}


#endif //FLY_FORSTMTDECL_H
