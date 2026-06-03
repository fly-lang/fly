//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTTestStmt.h - AST test block statement header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_TESTSTMT_H
#define FLY_AST_TESTSTMT_H

#include "ASTStmt.h"

namespace fly {

    class ASTBlockStmt;

    // Represents an inline "test { ... }" block inside a production function.
    // In non-test builds this node is created but stripped by the Resolver —
    // no Sema node and no IR are emitted. In test builds the body is resolved
    // normally with the three-scope read-only rules.
    class ASTTestStmt : public ASTStmt {

        friend class ASTBuilder;

        // The body of the test block
        ASTBlockStmt *Body = nullptr;

        explicit ASTTestStmt(const SourceLocation &Loc);

    public:

        void accept(ASTVisitor& Visitor) override;

        ASTBlockStmt *getBody() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_TESTSTMT_H
