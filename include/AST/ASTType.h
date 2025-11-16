//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTType.h - AST Identity Type header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_TYPE_H
#define FLY_AST_TYPE_H

#include "ASTNode.h"

namespace fly {

    class SemaType;
    class ASTExpr;

    enum class ASTBuiltinTypeKind {
		TYPE_VOID,
		TYPE_BOOL,
		TYPE_BYTE,
		TYPE_SHORT,
		TYPE_INT,
		TYPE_LONG,
		TYPE_USHORT,
		TYPE_UINT,
		TYPE_ULONG,
		TYPE_FLOAT,
		TYPE_DOUBLE,
		TYPE_STRING,
		TYPE_ERROR,
	};

    /**
     * Identity Type
     */
    class ASTType : public ASTNode {

        friend class ASTBuilder;

        SemaType *Sema;

    protected:

        explicit ASTType(const SourceLocation &Loc);

    public:

        SemaType *getSema() const;

        void setSema(SemaType *Sema);
    };

    class ASTBuiltinType : public ASTType {

        friend class ASTBuilder;

        ASTBuiltinTypeKind BuiltinKind;

        explicit ASTBuiltinType(const SourceLocation &Loc, ASTBuiltinTypeKind Kind);

    public:

        void accept(ASTVisitor& Visitor) override;

        std::string str() const override;

    };

    class ASTNamedType : public ASTType {

        friend class ASTBuilder;

        llvm::StringRef Name;

        explicit ASTNamedType(const SourceLocation &Loc, llvm::StringRef Name);

    public:

        void accept(ASTVisitor& Visitor) override;

        llvm::StringRef getName() const;

        std::string str() const override;
    };

    class ASTArrayType : public ASTType {

        friend class ASTBuilder;
        friend class Resolver;
        friend class SemaValidator;

        ASTType *ElementType;

        ASTExpr *Size;

        explicit ASTArrayType(const SourceLocation &Loc,  ASTType *ElementType, ASTExpr *Size);

    public:

        void accept(ASTVisitor& Visitor) override;

        ASTType *getElementType() const;

        ASTExpr *getSizeExpr() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_TYPE_H
