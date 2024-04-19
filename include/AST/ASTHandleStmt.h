//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTIfBlock.h - AST If Block Statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTHANDLESTMT_H
#define FLY_ASTHANDLESTMT_H

#include "ASTStmt.h"

namespace fly {

    class ASTVarRef;

    class ASTHandleStmt : public ASTStmt {

        friend class SemaBuilder;
        friend class SemaResolver;

        ASTVarRef *ErrorRef = nullptr;

        ASTStmt *Handle = nullptr;

        ASTHandleStmt(ASTStmt *Parent, const SourceLocation &Loc);

    public:

        ASTVarRef *getErrorRef() const;

        void setErrorRef(ASTVarRef *errorRef);

        ASTStmt *getHandle() const;

        void setHandle(ASTStmt *H);

        std::string str() const;
    };
}


#endif //FLY_ASTHANDLESTMT_H
