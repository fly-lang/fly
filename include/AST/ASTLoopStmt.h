//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTLoopStmt.h - AST loop statement header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_LOOPSTMT_H
#define FLY_AST_LOOPSTMT_H

#include "ASTRuleStmt.h"
#include "llvm/ADT/SmallVector.h"

namespace fly {

    class ASTBlockStmt;

    class ASTLoopStmt : public ASTStmt {

        friend class ASTBuilder;
        friend class ASTBuilderLoopStmt;

        bool VerifyConditionAtEnd = false;

    	ASTExpr *Expr = nullptr;

        llvm::SmallVector<ASTStmt *, 4> Init;

        llvm::SmallVector<ASTStmt *, 4> Post;

    	ASTStmt *Loop = nullptr;

        explicit ASTLoopStmt(const SourceLocation &Loc);

    public:

        void accept(ASTVisitor& Visitor) override;

        bool hasVerifyConditionAtEnd() const;

    	ASTExpr *getExpr() const;

        llvm::SmallVector<ASTStmt *, 4> &getInit();

        ASTStmt *getLoop() const;

        llvm::SmallVector<ASTStmt *, 4> &getPost();

        std::string str() const override;

    };
}

#endif //FLY_AST_LOOPSTMT_H
