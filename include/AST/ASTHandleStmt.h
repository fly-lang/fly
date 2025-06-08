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

    class ASTRef;
    class CodeGenHandle;

    class ASTHandleStmt : public ASTStmt {

        friend class ASTBuilder;
        friend class SemaBuilderStmt;
        friend class SemaResolver;
        friend class SemaValidator;

        ASTRef *ErrorHandlerRef = nullptr;

        ASTBlockStmt *Handle = nullptr;

        CodeGenHandle * CodeGen = nullptr;

        explicit ASTHandleStmt(const SourceLocation &Loc);

    public:

        ASTRef *getErrorHandlerRef() const;

        void setErrorHandlerRef(ASTRef *ErrorHandler);

        ASTBlockStmt* getHandle() const;

        CodeGenHandle *getCodeGen() const;

        void setCodeGen(CodeGenHandle *codeGen);

        std::string str() const override;
    };
}


#endif //FLY_AST_HANDLESTMT_H
