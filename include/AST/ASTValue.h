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

#include "ASTNode.h"

#include "llvm/ADT/StringMap.h"

namespace fly {

    class SourceLocation;
    class SemaValue;

    enum class ASTValueKind {
        VAL_BOOL,
        VAL_NUMBER,
        VAL_STRING,
        VAL_ARRAY,
        VAL_STRUCT,
        VAL_NULL,
    };


    class ASTValue : public ASTNode {

        friend class ASTBuilder;
        friend class SemaBuilder;
        friend class Resolver;
        friend class SemaValidator;

        const ASTValueKind ValueKind;

        SemaValue *Sema;

    protected:

        ASTValue(ASTValueKind ValueKind, const SourceLocation &Location);

    public:

        const ASTValueKind &getTypeKind() const;

        SemaValue *getSema() const {
			return Sema;
		}

    };

    /**
     * Used for Integer Numbers
     */
    class ASTBoolValue : public ASTValue {

        friend class ASTBuilder;
        friend class Resolver;
        friend class SemaValidator;

        bool Value;

        explicit ASTBoolValue(const SourceLocation &Loc, bool Value = false);

    public:

        bool getValue() const;

        std::string str() const override;
    };

    /**
     * Used for Numbers
     */
    class ASTNumberValue : public ASTValue {

        friend class ASTBuilder;
        friend class Resolver;
        friend class SemaValidator;

        llvm::StringRef Value; // the integer value

        ASTNumberValue(const SourceLocation &Loc, llvm::StringRef Value);

    public:

        llvm::StringRef getValue() const;

        std::string str() const override;
    };

    /**
     * Used for String
     */
    class ASTStringValue : public ASTValue {

        friend class ASTBuilder;
        friend class Resolver;
        friend class SemaValidator;

        llvm::StringRef Value;

        ASTStringValue(const SourceLocation &Loc, llvm::StringRef Value);

    public:

        llvm::StringRef getValue() const;

        std::string str() const override;
    };

    /**
     * Used for Arrays
     */
    class ASTArrayValue : public ASTValue {

        friend class ASTBuilder;
        friend class Resolver;
        friend class SemaValidator;

        llvm::SmallVector<ASTValue *, 8> Values;

        explicit ASTArrayValue(const SourceLocation &Loc);

    public:

        const llvm::SmallVector<ASTValue *, 8> &getValues() const;

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
        friend class SemaValidator;

        llvm::StringMap<ASTValue *> Values;

        explicit ASTStructValue(const SourceLocation &Loc);

    public:

        const llvm::StringMap<ASTValue *> &getValues() const;

        size_t size() const;

        bool empty() const;

        std::string str() const override;
    };

    class ASTNullValue : public ASTValue {

        friend class ASTBuilder;
        friend class Resolver;
        friend class SemaValidator;

        explicit ASTNullValue(const SourceLocation &Loc);

    public:

        std::string str() const override;
    };

}

#endif //FLY_AST_VALUE_H
