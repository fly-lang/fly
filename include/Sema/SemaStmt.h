//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaStmt.h - statement semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_STMT_H
#define FLY_SEMA_STMT_H

#include "Sema/SemaNode.h"

namespace fly {

    class ASTStmt;

    class SemaStmt : public SemaNode {

        ASTStmt *AST;

    protected:

        explicit SemaStmt(SemaKind Kind, ASTStmt *AST);

    public:

        ~SemaStmt() override = default;

        ASTStmt *getAST() const;

        void accept(SemaVisitor &Visitor) override = 0;
    };
}

#endif //FLY_SEMA_STMT_H

