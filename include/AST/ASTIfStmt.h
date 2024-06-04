//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTIfBlock.h - AST If Block Statement header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_IFSTMT_H
#define FLY_AST_IFSTMT_H

#include "ASTStmt.h"

namespace fly {

    class ASTIfStmt;
    class ASTElsif;

    class ASTIfStmt : public ASTStmt {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        // The If expression condition
        ASTExpr *Condition = nullptr;

        // The If Block statement
        ASTStmt *Stmt = nullptr;

        // The list of Elseif Blocks
        llvm::SmallVector<ASTElsif *, 8> Elsif;

        // The Else Block
        ASTStmt *Else = nullptr;

        explicit ASTIfStmt(const SourceLocation &Loc);

    public:

        ASTExpr *getCondition();

        ASTStmt *getStmt() const;

        llvm::SmallVector<ASTElsif *, 8> getElsif();

        ASTStmt *getElse();

        std::string str() const override;
    };

    class ASTElsif {

        friend class SemaBuilder;
        friend class SemaResolver;

        // The Else If expression condition
        ASTExpr *Condition = nullptr;

        // The Elsif Block statement
        ASTBlockStmt *Block = nullptr;

        explicit ASTElsif(const SourceLocation &Loc);

    public:

        ASTExpr *getCondition();

        ASTBlockStmt *getBlock() const;

        std::string str() const;
    };
}


#endif //FLY_AST_IFSTMT_H
