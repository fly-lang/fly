//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaCaseStmt.h - suite case statement semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_CASESTMT_H
#define FLY_SEMA_CASESTMT_H

#include "Sema/SemaStmt.h"
#include <string>

namespace fly {

    class SemaBlockStmt;
    class ASTCaseStmt;

    // Resolved "case" statement in a suite test-method body.
    // Each case executes sequentially — no dispatch, no break.
    class SemaCaseStmt : public SemaStmt {

        friend class Resolver;

        std::string Label;
        SemaBlockStmt *Body = nullptr;

        SemaCaseStmt(ASTCaseStmt *AST, std::string Label, SemaBlockStmt *Body);

    public:

        ~SemaCaseStmt() override = default;

        const std::string &getLabel() const;

        SemaBlockStmt *getBody() const;

        void accept(SemaVisitor &Visitor) override;

        std::string str() const override;
    };
}

#endif //FLY_SEMA_CASESTMT_H
