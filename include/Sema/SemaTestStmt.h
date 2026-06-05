//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaTestStmt.h - test block statement semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_TESTSTMT_H
#define FLY_SEMA_TESTSTMT_H

#include "Sema/SemaStmt.h"

namespace fly {

    class SemaBlockStmt;
    class ASTTestStmt;

    // Resolved "test { ... }" block. Only created in test mode; the block's
    // content is resolved with three-scope read-only access rules.
    class SemaTestStmt : public SemaStmt {

        friend class Resolver;

        SemaBlockStmt *Body = nullptr;

        explicit SemaTestStmt(ASTTestStmt *AST);

    public:

        ~SemaTestStmt() override = default;

        SemaBlockStmt *getBody() const;

        void accept(SemaVisitor &Visitor) override;

        std::string str() const override;
    };
}

#endif //FLY_SEMA_TESTSTMT_H
