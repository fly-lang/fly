//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTValue.h - AST literal value header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_VALUE_H
#define FLY_AST_VALUE_H

#include "ASTExpr.h"
#include "llvm/ADT/StringMap.h"

namespace fly {

    enum class ASTValueKind {
        VAL_BOOL,
        VAL_NUMBER,
        VAL_STRING,
        VAL_ARRAY,
        VAL_STRUCT,
        VAL_NULL,
        VAL_DEFAULT,
        VAL_ENUM,
        VAL_UNSET
    };

    class ASTValue : public ASTExpr {

        friend class ASTBuilder;

        const ASTValueKind ValueKind;

    protected:

        ASTValue(ASTValueKind ValueKind, const SourceLocation &Location);

    public:

        const ASTValueKind &getValueKind() const;


        // Predicate helpers
        bool isBool() const;
        bool isNumber() const;
        bool isString() const;
        bool isArray() const;
        bool isStruct() const;
        bool isNull() const;
        bool isDefault() const;
        bool isUnset() const;

    };

    /**
     * Used for Integer Numbers
     */
    class ASTBoolValue : public ASTValue {

        friend class ASTBuilder;
        friend class Resolver;

        bool Value;

        explicit ASTBoolValue(const SourceLocation &Loc, bool Value);

    public:

        void accept(ASTVisitor& Visitor) override;

        bool getValue() const;

        std::string str() const override;
    };

    /**
     * Used for Numbers
     */
    class ASTNumberValue : public ASTValue {

        friend class ASTBuilder;
        friend class Resolver;

        llvm::StringRef Value; // the integer value

        ASTNumberValue(const SourceLocation &Loc, llvm::StringRef Value);

    public:

        void accept(ASTVisitor& Visitor) override;

        llvm::StringRef getValue() const;

        std::string str() const override;
    };

    /**
     * Used for String
     */
    class ASTStringValue : public ASTValue {

        friend class ASTBuilder;
        friend class Resolver;

        llvm::StringRef Value;

        ASTStringValue(const SourceLocation &Loc, llvm::StringRef Value);

    public:

        void accept(ASTVisitor& Visitor) override;

        llvm::StringRef getValue() const;

        std::string str() const override;
    };

    /**
     * Used for Arrays
     */
    class ASTArrayValue : public ASTValue {

        friend class ASTBuilder;
        friend class Resolver;

        // Elements are EXPRESSIONS (a literal is-an expression): array literals
        // accept casts, identifiers and arithmetic — `{(byte)200, a + 1}` —
        // not just bare values (B026).
        llvm::SmallVector<ASTExpr *, 8> Values;

        explicit ASTArrayValue(const SourceLocation &Loc);

    public:

        ~ASTArrayValue() override;

        void accept(ASTVisitor& Visitor) override;

        const llvm::SmallVector<ASTExpr *, 8> &getValues() const;

        size_t size() const;

        bool empty() const;

        std::string str() const override;
    };

    /**
     * Used for Structs
     */
    class ASTStructValue : public ASTValue {

        friend class ASTBuilder;
        friend class Resolver;

        // Field values are EXPRESSIONS, like array elements (B029 parity):
        // `{x = base + 1}` is as valid as `{x = 1}`.
        llvm::StringMap<ASTExpr *> Values;

        explicit ASTStructValue(const SourceLocation &Loc);

    public:

        ~ASTStructValue() override;

        void accept(ASTVisitor& Visitor) override;

        const llvm::StringMap<ASTExpr *> &getValues() const;

        size_t size() const;

        bool empty() const;

        std::string str() const override;
    };

    class ASTNullValue : public ASTValue {

        friend class ASTBuilder;
        friend class Resolver;

        explicit ASTNullValue(const SourceLocation &Loc);

    public:

        void accept(ASTVisitor& Visitor) override;

        std::string str() const override;
    };

    /**
     * Represents an unset enum value (index 0)
     */
    class ASTUnsetValue : public ASTValue {

        friend class ASTBuilder;
        friend class Resolver;

        explicit ASTUnsetValue(const SourceLocation &Loc);

    public:

        void accept(ASTVisitor& Visitor) override;

        std::string str() const override;
    };

}

#endif //FLY_AST_VALUE_H
