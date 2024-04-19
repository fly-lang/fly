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

#include "ASTBase.h"

namespace fly {

    class ASTEnumType;
    class ASTClassType;
    class ASTIdentity;
    class ASTEnum;
    class ASTClass;

    enum class ASTErrorKind {
        ERR_NONE = 0,
        ERR_INT = 1,
        ERR_STRING = 2,
        ERR_ENUM = 3,
        ERR_CLASS = 4
    };

    class ASTError : public ASTBase {

        friend class SemaBuilder;
        friend class SemaResolver;

        const ASTErrorKind ErrorKind;

        const uint32_t Code;

        const llvm::StringRef Message;

        const ASTIdentity *Identity;

    protected:

        ASTError(const SourceLocation &Loc, uint32_t Code);

        ASTError(const SourceLocation &Loc, llvm::StringRef Message);

        ASTError(const SourceLocation &Loc, ASTEnum *Enum);

        ASTError(const SourceLocation &Loc, ASTClass *Class);

    public:

        ASTErrorKind getErrorKind() const;

        uint32_t getCode() const;

        llvm::StringRef getMessage() const;

        ASTEnum* getEnum() const;

        ASTClass* getClass() const;

        std::string print() const;

        std::string str() const;

    };
}

#endif //FLY_AST_ERROR_H
