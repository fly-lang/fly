//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTType.h - AST type reference header
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

    struct Symbol;
    class ASTName;
    class ASTExpr;

    enum class ASTTypeKind {
        TYPE_NAMED,
        TYPE_BUILTIN,
        TYPE_ARRAY
    };

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
		TYPE_COMPLEX,
		TYPE_STRING,
		TYPE_ERROR,
	};

    /**
     * Identity Type
     */
    class ASTType : public ASTNode {

        friend class ASTBuilder;

        ASTTypeKind TypeKind;

        Symbol *Sym = nullptr;

    protected:

        explicit ASTType(const SourceLocation &Loc, ASTTypeKind TypeKind);

    public:

        ASTTypeKind getTypeKind() const;

        Symbol *getSymbol() const;

        void setSymbol(Symbol *Sym);
    };

    class ASTBuiltinType : public ASTType {

        friend class ASTBuilder;

        ASTBuiltinTypeKind BuiltinKind;

        explicit ASTBuiltinType(const SourceLocation &Loc, ASTBuiltinTypeKind Kind);

    public:

        void accept(ASTVisitor& Visitor) override;

        ASTBuiltinTypeKind getBuiltinKind() const;

        std::string str() const override;

    };

    class ASTNamedType : public ASTType {

        friend class ASTBuilder;

        llvm::SmallVector<ASTName *, 4> Names;

        explicit ASTNamedType(const SourceLocation &Loc, llvm::SmallVector<ASTName *, 4> Names);

    public:

        void accept(ASTVisitor& Visitor) override;

        const llvm::SmallVector<ASTName*, 4>& getNames() const;

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
