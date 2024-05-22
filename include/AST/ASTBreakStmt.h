//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTBlock.h - AST Block Statement header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_BREAKSTMT_H
#define FLY_AST_BREAKSTMT_H

#include "ASTStmt.h"

namespace fly {

    /**
     * Break Stmt
     */
    class ASTBreakStmt : public ASTStmt {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

    public:
        explicit ASTBreakStmt(const SourceLocation &Loc);

        std::string str() const override;
    };
}

#endif //FLY_AST_BREAKSTMT_H
