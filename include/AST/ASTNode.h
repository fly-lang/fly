//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTNode.h - AST Base header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_BASE_H
#define FLY_AST_BASE_H

#include <Basic/Logger.h>

#include "Basic/SourceLocation.h"
#include "llvm/ADT/SmallVector.h"

namespace fly {

    class ASTVisitor;

    enum class ASTKind {
        AST_MODULE,
        AST_NAMESPACE,
        AST_IMPORT,
        AST_VAR,
        AST_ARG,
        AST_STMT,
        AST_TYPE,
        AST_VALUE,
        AST_EXPR,
        AST_REF,
        AST_COMMENT,
        AST_FUNCTION,
        AST_MODIFIER,
        AST_STRUCT,
        AST_CLASS,
        AST_INTERFACE,
        AST_ENUM,
    };

    class ASTNode {

        friend class ASTBuilder;
        friend class Resolver;
        friend class SemaValidator;

        SourceLocation Location;

        ASTKind Kind;

    public:
        virtual ~ASTNode() = default;

        explicit ASTNode(const SourceLocation &Loc, ASTKind Kind);

        virtual void accept(ASTVisitor& v) = 0;

        virtual const SourceLocation &getLocation() const;

        virtual ASTKind getKind() const;

        virtual std::string str() const;

        template <typename T>
        static const std::string &str(llvm::SmallVector<T *, 8> Vect) {
            std::string Str = Logger::OPEN_LIST;
            if(!Vect.empty()) {
                for (ASTNode *V : Vect) {
                    Str += (V ? V->str() : "") + Logger::SEP;
                }
                unsigned long end = Str.length()-std::string(Logger::SEP).length()-1;
                Str = Str.substr(0, end);
            }
            Str += Logger::CLOSE_LIST;
            return Str;
        }

    };

}

#endif //FLY_AST_BASE_H