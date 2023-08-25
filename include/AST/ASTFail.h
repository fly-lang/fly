//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTFail.h - Fail declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_AST_FAIL_H
#define FLY_AST_FAIL_H

#include "ASTExprStmt.h"
#include "Basic/SourceLocation.h"

namespace fly {

    class ASTClassType;

    enum class ASTFailKind {
        ERR_NUMBER = 1,
        ERR_MESSAGE = 2,
        ERR_CLASS = 3
    };

    class ASTFail : public ASTStmt  {

        friend class SemaBuilder;
        friend class SemaResolver;

    protected:

        const llvm::StringRef Message;

        const uint32_t Code;

        const ASTClassType *Class;

        const ASTFailKind FailKind;

        ASTFail(ASTStmt *Parent, const SourceLocation &Loc, uint32_t Code);

        ASTFail(ASTStmt *Parent, const SourceLocation &Loc, llvm::StringRef Message);

        ASTFail(ASTStmt *Parent, const SourceLocation &Loc, ASTClassType *Class);

        ~ASTFail();

    public:

        uint32_t getCode() const;

        llvm::StringRef getMessage() const;

        ASTFailKind getFailKind() const;

        std::string print() const;

        std::string str() const;
    };
}

#endif //FLY_AST_FAIL_H
