//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTTypeRef.h - AST Identity Type header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_TYPEREF_H
#define FLY_AST_TYPEREF_H

#include "ASTRef.h"

namespace fly {

    class SemaType;
    class ASTExpr;
    class ASTNameSpaceRef;

    /**
     * Identity Type
     */
    class ASTTypeRef : public ASTRef {

        friend class ASTBuilder;
        friend class Resolver;
        friend class SemaValidator;

        SemaType *Sema;

        bool Array;

        ASTNameSpaceRef *NameSpaceRef;

    protected:

        explicit ASTTypeRef(const SourceLocation &Loc, llvm::StringRef Name, ASTNameSpaceRef *NameSpaceRef = nullptr,
            bool Array = false);

    public:

        SemaType *getSema() const;

        bool isArray() const;

        ASTNameSpaceRef *getNameSpaceRef() const;

        std::string str() const override;
    };

    class ASTArrayTypeRef : public ASTTypeRef {

        friend class ASTBuilder;
        friend class Resolver;
        friend class SemaValidator;

        ASTTypeRef *TypeRef;

        explicit ASTArrayTypeRef(const SourceLocation &Loc,  ASTTypeRef *TypeRef, llvm::StringRef Name);

    public:

        ASTTypeRef *getTypeRef() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_TYPEREF_H
