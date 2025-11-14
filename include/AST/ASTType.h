//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTType.h - AST Identity Type header
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

    class SemaType;
    class ASTExpr;

    /**
     * Identity Type
     */
    class ASTType : public ASTIdentifier {

        friend class ASTBuilder;

        SemaType *Sema;

        bool BuiltIn;

    protected:

        explicit ASTType(const SourceLocation &Loc, llvm::StringRef Name, bool BuiltIn = false);

    public:

        void accept(ASTVisitor& Visitor) override;

        bool isBuiltIn() const;

        SemaType *getSema() const;

        void setSema(SemaType *Sema);

        std::string str() const override;
    };

    class ASTArrayType : public ASTType {

        friend class ASTBuilder;
        friend class Resolver;
        friend class SemaValidator;

        ASTType *ElementType;

        ASTExpr *SizeExpr;

        explicit ASTArrayType(const SourceLocation &Loc,  ASTType *ElementType, ASTExpr *SizeExpr, llvm::StringRef Name);

    public:

        void accept(ASTVisitor& Visitor) override;

        ASTType *getElementType() const;

        ASTExpr *getSizeExpr() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_TYPEREF_H
