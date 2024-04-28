//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTFailStmt.h - AST Fail Statement header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_FAILSTMT_H
#define FLY_AST_FAILSTMT_H

#include "ASTStmt.h"

namespace fly {

    class ASTVarRef;
    class ASTHandleStmt;

    class ASTFailStmt : public ASTStmt {

        friend class SemaBuilder;

        ASTExpr *Expr = nullptr;

        ASTHandleStmt *Handle = nullptr;

        ASTFailStmt(ASTStmt *Parent, const SourceLocation &Loc);

    public:

        ASTExpr *getExpr() const;

        void setExpr(ASTExpr *);

        bool hasHandle();

        ASTHandleStmt *getHandle();

        std::string str() const override;
    };
}

#endif //FLY_AST_FAILSTMT_H
