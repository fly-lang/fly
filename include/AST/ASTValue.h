//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTValue.h - AST Value header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_VALUE_H
#define FLY_AST_VALUE_H

#include "ASTBase.h"

#include "llvm/ADT/StringMap.h"

namespace fly {

    class ASTType;
    class SourceLocation;

    enum class ASTTypeKind;

    class ASTValue : public ASTBase {

        friend class SemaBuilder;
        friend class SemaResolver;

        const ASTTypeKind TypeKind;

    protected:

        ASTValue(ASTTypeKind TypeKind, const SourceLocation &Location);

    public:

        const ASTTypeKind &getTypeKind() const;

        std::string printType() const;

        virtual std::string print() const = 0;

        std::string str() const override;
    };

    /**
     * Used for Integer Numbers
     */
    class ASTBoolValue : public ASTValue {

        friend class SemaBuilder;

        bool Value;

        explicit ASTBoolValue(const SourceLocation &Loc, bool Value = false);

    public:

        bool getValue() const;

        std::string print() const override;

        std::string str() const override;
    };

    /**
     * Used for Integer Numbers
     */
    class ASTIntegerValue : public ASTValue {

        friend class SemaBuilder;
        friend class SemaResolver;

        uint64_t Value; // the integer value

        bool Negative; // true is positive, false is negative

        ASTIntegerValue(const SourceLocation &Loc, uint64_t Value, bool Negative = false);

    public:

        bool isNegative() const;

        uint64_t getValue() const;

        std::string print() const override;

        std::string str() const override;
    };

    /**
     * Used for Floating Point Numbers
     */
    class ASTFloatingValue : public ASTValue {

        friend class SemaBuilder;
        friend class SemaResolver;

        std::string Value;

        ASTFloatingValue(const SourceLocation &Loc, std::string Val);

    public:

        std::string getValue() const;

        std::string print() const override;

        std::string str() const override;
    };

    /**
     * Used for String
     */
    class ASTStringValue : public ASTValue {

        friend class SemaBuilder;

        llvm::StringRef Value;

        ASTStringValue(const SourceLocation &Loc, llvm::StringRef Value);

    public:

        llvm::StringRef getValue() const;

        std::string print() const override;

        std::string str() const override;
    };

    /**
     * Used for Arrays
     */
    class ASTArrayValue : public ASTValue {

        friend class SemaBuilder;

        llvm::SmallVector<ASTValue *, 8> Values;

        explicit ASTArrayValue(const SourceLocation &Loc);

    public:

        const llvm::SmallVector<ASTValue *, 8> &getValues() const;

        uint64_t size() const;

        bool empty() const;

        std::string print() const override;

        std::string str() const override;
    };

    /**
     * Used for Structs
     */
    class ASTStructValue : public ASTValue {

        friend class SemaBuilder;

        llvm::StringMap<ASTValue *> Values;

        explicit ASTStructValue(const SourceLocation &Loc);

    public:

        const llvm::StringMap<ASTValue *> &getValues() const;

        uint64_t size() const;

        bool empty() const;

        std::string print() const override;

        std::string str() const override;
    };

    class ASTNullValue : public ASTValue {

        friend class SemaBuilder;

        explicit ASTNullValue(const SourceLocation &Loc);

    public:

        std::string print() const override;

        std::string str() const override;
    };

    class ASTZeroValue : public ASTValue {

        friend class SemaBuilder;

        explicit ASTZeroValue(const SourceLocation &Loc);

    public:

        std::string print() const override;

        std::string str() const override;
    };
}

#endif //FLY_AST_VALUE_H
