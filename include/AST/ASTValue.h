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

    class SourceLocation;

    enum class ASTValueKind {
        VAL_BOOL,
        VAL_INT,
        VAL_FLOAT,
        VAL_STRING,
        VAL_CHAR,
        VAL_ARRAY,
        VAL_STRUCT,
        VAL_NULL,
        VAL_ZERO
    };


    class ASTValue : public ASTBase {

        friend class ASTBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        const ASTValueKind ValueKind;

    protected:

        ASTValue(ASTValueKind ValueKind, const SourceLocation &Location);

    public:

        const ASTValueKind &getTypeKind() const;

        std::string str() const override;
    };

    /**
     * Used for Integer Numbers
     */
    class ASTBoolValue : public ASTValue {

        friend class ASTBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        bool Value;

        explicit ASTBoolValue(const SourceLocation &Loc, bool Value = false);

    public:

        bool getValue() const;

        std::string print() const;

        std::string str() const override;
    };

    /**
     * Used for Integer Numbers
     */
    class ASTIntegerValue : public ASTValue {

        friend class ASTBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        llvm::StringRef Value; // the integer value

        uint8_t Radix;

        ASTIntegerValue(const SourceLocation &Loc, llvm::StringRef Value, uint8_t Radix);

    public:

        llvm::StringRef getValue() const;

        uint8_t getRadix() const;

        std::string str() const override;
    };

    /**
     * Used for Floating Point Numbers
     */
    class ASTFloatingValue : public ASTValue {

        friend class ASTBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        std::string Value;

        ASTFloatingValue(const SourceLocation &Loc, llvm::StringRef Val);

    public:

        std::string getValue() const;

        std::string print() const;

        std::string str() const override;
    };

    /**
     * Used for Char
     */
    class ASTCharValue : public ASTValue {

        friend class ASTBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        llvm::StringRef Value;

        ASTCharValue(const SourceLocation &Loc, llvm::StringRef Value);

    public:

        llvm::StringRef getValue() const;

        std::string str() const override;
    };

    /**
     * Used for String
     */
    class ASTStringValue : public ASTValue {

        friend class ASTBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        llvm::StringRef Value;

        ASTStringValue(const SourceLocation &Loc, llvm::StringRef Value);

    public:

        llvm::StringRef getValue() const;

        std::string print() const;

        std::string str() const override;
    };

    /**
     * Used for Arrays
     */
    class ASTArrayValue : public ASTValue {

        friend class ASTBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        llvm::SmallVector<ASTValue *, 8> Values;

        explicit ASTArrayValue(const SourceLocation &Loc);

    public:

        const llvm::SmallVector<ASTValue *, 8> &getValues() const;

        size_t size() const;

        bool empty() const;

        std::string print() const;

        std::string str() const override;
    };

    /**
     * Used for Structs
     */
    class ASTStructValue : public ASTValue {

        friend class ASTBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        llvm::StringMap<ASTValue *> Values;

        explicit ASTStructValue(const SourceLocation &Loc);

    public:

        const llvm::StringMap<ASTValue *> &getValues() const;

        size_t size() const;

        bool empty() const;

        std::string print() const;

        std::string str() const override;
    };

    class ASTNullValue : public ASTValue {

        friend class ASTBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        explicit ASTNullValue(const SourceLocation &Loc);

    public:

        std::string print() const;

        std::string str() const override;
    };

    class ASTZeroValue : public ASTValue {

        friend class ASTBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        explicit ASTZeroValue(const SourceLocation &Loc);

    public:

        std::string print() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_VALUE_H
