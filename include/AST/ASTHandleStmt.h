//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTHandleStmt.h - AST Handle Statement header
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
    class CodeGenHandle;

    class ASTHandleStmt : public ASTStmt {

        friend class ASTBuilder;

        ASTExpr *ErrorHandler = nullptr;

        ASTBlockStmt *Handle = nullptr;

        explicit ASTHandleStmt(const SourceLocation &Loc);

    public:

        void accept(ASTVisitor& Visitor) override;

        ASTExpr *getErrorHandler() const;

        void setErrorHandler(ASTExpr *ErrorHandler);

        ASTBlockStmt* getHandle() const;

        std::string str() const override;
    };
}


#endif //FLY_AST_HANDLESTMT_H
