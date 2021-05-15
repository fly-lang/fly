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

        friend class Parser;

        enum StmtKind StmtKind = StmtKind::D_STMT_FOR;

        StmtDecl *Init;
        GroupExpr *Cond;
        StmtDecl *Post;

    public:
        ForStmtDecl(const SourceLocation &Loc, StmtDecl *Parent);

        enum StmtKind getStmtKind() const override;

        const StmtDecl *getInit() const;

        const GroupExpr *getCondition() const;

        const StmtDecl *getPost() const;

    };
}


#endif //FLY_FORSTMTDECL_H
