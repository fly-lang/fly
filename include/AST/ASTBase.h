//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTBase.h - AST base node header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_BASE_H
#define FLY_AST_BASE_H

#include "Basic/SourceLocation.h"
#include <Basic/Logger.h>

namespace fly {

    class SourceLocation;

    enum class ASTKind
    {
        AST_MODULE,
        AST_NAMESPACE,
        AST_IMPORT,
        AST_NAME,
        AST_VAR,
        AST_ARG,
        AST_STMT,
        AST_TYPE,
        AST_EXPR,
        AST_COMMENT,
        AST_FUNCTION,
        AST_MODIFIER,
        AST_CLASS,
        AST_METHOD,
        AST_ENUM,
        AST_VALUE
    };

    class ASTBase
    {
        SourceLocation Loc;

        ASTKind Kind;

    protected:
        ASTBase(const SourceLocation& Loc, ASTKind Kind);

    public:
        virtual ~ASTBase() = default;

        virtual const SourceLocation& getLocation() const;

        virtual ASTKind getKind() const;

        virtual void setKind(ASTKind Kind);

        virtual std::string str() const;

        template <typename T>
        static std::string str(const llvm::SmallVector<T*, 8>& Vect)
        {
            std::string S;
            S.reserve(128);
            S += Logger::OPEN_LIST;

            if (!Vect.empty())
            {
                for (auto* V : Vect)
                {
                    S += (V ? V->str() : "null");
                    S += Logger::SEP;
                }
                // Remove the last separator
                S.resize(S.size() - std::string(Logger::SEP).size());
            }

            S += Logger::CLOSE_LIST;
            return S;
        }
    };
}

#endif //FLY_AST_BASE_H
