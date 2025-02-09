//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTBase.h - AST Base header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_BASE_H
#define FLY_AST_BASE_H

#include "Sema/Logger.h"
#include "Basic/SourceLocation.h"

namespace fly {

    enum class ASTKind {
        AST_NAMESPACE,
        AST_IMPORT,
        AST_ALIAS,
        AST_VAR,
        AST_ARG,
        AST_STMT,
        AST_TYPE,
        AST_VALUE,
        AST_EXPR,
        AST_IDENTIFIER,
        AST_COMMENT,
        AST_FUNCTION,
        AST_SCOPE,
        AST_STRUCT,
        AST_CLASS,
        AST_INTERFACE,
        AST_ENUM,
    };

    class ASTBase {

        friend class ASTBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        SourceLocation Location;

        ASTKind Kind;

    public:
        virtual ~ASTBase() = default;

        explicit ASTBase(const SourceLocation &Loc, ASTKind Kind);

        virtual const SourceLocation &getLocation() const;

        virtual ASTKind getKind() const;

        virtual std::string str() const = 0;
    };

}

#endif //FLY_AST_BASE_H