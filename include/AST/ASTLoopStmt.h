//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTWhileBlock.h - AST While Block Statement header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_LOOPSTMT_H
#define FLY_AST_LOOPSTMT_H

#include "ASTStmt.h"

namespace fly {

    class ASTLoopStmt : public ASTStmt {

        friend class SemaBuilder;
        friend class SemaResolver;

        ASTExpr *Condition = nullptr;

        bool VerifyConditionOnEnd = false;

        ASTBlockStmt *Block = nullptr;

        ASTBlockStmt *Init = nullptr;

        ASTBlockStmt *Post = nullptr;

        explicit ASTLoopStmt(const SourceLocation &Loc);

    public:

        ASTExpr *getCondition();

        bool isVerifyConditionOnEnd() const;

        ASTBlockStmt *getBlock() const;

        ASTBlockStmt *getInit() const;

        ASTBlockStmt *getPost() const;

        std::string str() const override;

    };
}

#endif //FLY_AST_LOOPSTMT_H
