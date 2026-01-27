//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTLoopInStmt.h - AST Loop In Statement header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_LOOPINSTMT_H
#define FLY_AST_LOOPINSTMT_H

#include "ASTStmt.h"
#include "AST/ASTExpr.h"

namespace fly {

    class ASTLoopInStmt : public ASTStmt {

        friend class ASTBuilder;
        friend class ASTBuilderLoopInStmt;

        ASTExpr *Item = nullptr;

        ASTExpr *List = nullptr;

        ASTStmt *Stmt = nullptr;

        explicit ASTLoopInStmt(const SourceLocation &Loc, ASTExpr *Item, ASTExpr *List, ASTStmt *Stmt);

    public:

        void accept(ASTVisitor& Visitor) override;

        ASTExpr *getItem() const;

        ASTExpr *getList() const;

        ASTStmt *getStmt() const;

        std::string str() const override;

    };
}

#endif //FLY_AST_LOOPINSTMT_H
