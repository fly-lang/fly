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

#include "ASTIdentifier.h"

namespace fly {

    class SymType;
    class ASTExpr;

    /**
     * Identity Type
     */
    class ASTTypeRef : public ASTIdentifier {

        friend class ASTBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        SymType *Def;

        bool Array;

    protected:

        explicit ASTTypeRef(const SourceLocation &Loc, llvm::StringRef Name, bool Array = false);

    public:

        SymType *getDef() const;

        bool isArray() const;

        std::string str() const override;
    };

    class ASTArrayTypeRef : public ASTTypeRef {

        friend class ASTBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        ASTTypeRef *Type;

        ASTExpr *Size;

        explicit ASTArrayTypeRef(const SourceLocation &Loc, llvm::StringRef Name);

    public:

        ASTExpr *getSize() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_TYPEREF_H
