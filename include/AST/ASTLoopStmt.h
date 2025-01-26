//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTLoopStmt.h - AST Loop Statement header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_LOOPSTMT_H
#define FLY_AST_LOOPSTMT_H

#include "ASTRuleStmt.h"

namespace fly {

    class ASTLoopStmt : public ASTRuleStmt {

        friend class SemaBuilder;
        friend class SemaBuilderLoopStmt;
        friend class SemaResolver;
        friend class SemaValidator;

        bool VerifyConditionAtEnd = false;

        ASTStmt *Init = nullptr;

        ASTStmt *Post = nullptr;

        explicit ASTLoopStmt(const SourceLocation &Loc);

    public:

        bool hasVerifyConditionAtEnd() const;

        ASTStmt *getInit() const;

        ASTStmt *getLoop() const;

        ASTStmt *getPost() const;

        std::string str() const override;

    };
}

#endif //FLY_AST_LOOPSTMT_H
