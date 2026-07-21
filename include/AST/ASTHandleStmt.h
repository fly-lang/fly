//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTHandleStmt.h - AST handle statement header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_HANDLESTMT_H
#define FLY_AST_HANDLESTMT_H

#include "ASTStmt.h"

namespace fly {

    class ASTIdentifier;
    class ASTLocalVar;

    class ASTHandleStmt : public ASTStmt {

        friend class ASTBuilder;

        ASTBlockStmt *Handle = nullptr;

        // "error err handle { ... }": the declared var bound to this handle's
        // error handler. Null for the anonymous "handle { ... }" form.
        ASTLocalVar *ErrorVar = nullptr;

        explicit ASTHandleStmt(const SourceLocation &Loc);

    public:

        void accept(ASTVisitor& Visitor) override;

        ASTBlockStmt* getHandle() const;

        ASTLocalVar *getErrorVar() const;

        void setErrorVar(ASTLocalVar *V);

        std::string str() const override;
    };
}

#endif //FLY_AST_HANDLESTMT_H
