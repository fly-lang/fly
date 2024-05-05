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

    class ASTVarRef;
    class CodeGenHandle;

    class ASTHandleStmt : public ASTStmt {

        friend class SemaBuilder;
        friend class SemaResolver;

        ASTVarRef *ErrorHandlerRef = nullptr;

        ASTStmt *Handle = nullptr;

        CodeGenHandle * CodeGen = nullptr;

        explicit ASTHandleStmt(const SourceLocation &Loc);

    public:

        ASTVarRef *getErrorHandlerRef() const;

        void setErrorHandlerRef(ASTVarRef *errorRef);

        ASTStmt *getHandle() const;

        void setHandle(ASTStmt *H);

        CodeGenHandle *getCodeGen() const;

        void setCodeGen(CodeGenHandle *codeGen);

        std::string str() const override;
    };
}


#endif //FLY_AST_HANDLESTMT_H
