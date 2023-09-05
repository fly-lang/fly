//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTError.h - Error declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_AST_ERROR_H
#define FLY_AST_ERROR_H

#include "ASTExprStmt.h"
#include "Basic/SourceLocation.h"

namespace fly {

    class ASTClassType;

    enum class ASTErrorKind {
        ERR_ENABLED = 0,
        ERR_CODE = 1,
        ERR_MESSAGE = 2,
        ERR_CLASS = 3
    };

    class ASTError {

        friend class SemaBuilder;
        friend class SemaResolver;

        const ASTErrorKind ErrorKind;

        const SourceLocation Loc;

        const bool Enabled;

        const uint32_t Code;

        const llvm::StringRef Message;

        const ASTClassType *Class;

    protected:

        ASTError(const SourceLocation &Loc, uint32_t Code);

        ASTError(const SourceLocation &Loc, llvm::StringRef Message);

        ASTError(const SourceLocation &Loc, ASTClassType *Class);

    public:

        ASTErrorKind getErrorKind() const;

        const SourceLocation &getLocation() const;

        bool isEnabled() const;

        uint32_t getCode() const;

        llvm::StringRef getMessage() const;

        std::string print() const;

        std::string str() const;

    };
}

#endif //FLY_AST_ERROR_H
