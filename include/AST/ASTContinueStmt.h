//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTContinueStmt.h - AST Block Statement header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_CONTINUESTMT_H
#define FLY_AST_CONTINUESTMT_H

#include "ASTStmt.h"

namespace fly {

    /**
     * Continue Stmt
     */
    class ASTContinueStmt : public ASTStmt {

    public:
        explicit ASTContinueStmt(const SourceLocation &Loc);

        std::string str() const override;
    };
}

#endif //FLY_AST_CONTINUESTMT_H
