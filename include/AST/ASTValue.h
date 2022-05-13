//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTValue.h - AST Value
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_ASTVALUE_H
#define FLY_ASTVALUE_H

#include <string>
#include <vector>

namespace fly {

    class ASTType;
    class SourceLocation;

    class ASTValue {

        const SourceLocation &Location;

        ASTType *Type;

    public:
        ASTValue(const SourceLocation &Location);

        const SourceLocation &getLocation() const;

        ASTType *getType() const;

        virtual std::string str() const = 0;
    };

    /**
     * Abstract Value for Integer, Floating Point and Boolean
     */
    class ASTSingleValue : public ASTValue {

    public:
        ASTSingleValue(const SourceLocation &Loc);
    };

    /**
     * Used for Integer Numbers
     */
    class ASTBoolValue : public ASTSingleValue {

        bool Value;

    public:
        ASTBoolValue(const SourceLocation &Loc, bool Value = false);

        bool getValue() const;

        std::string str() const override;
    };

    /**
     * Used for Integer Numbers
     */
    class ASTIntegerValue : public ASTSingleValue {

        uint64_t Value; // the integer value

        bool Negative; // true is positive, false is negative

    public:
        ASTIntegerValue(const SourceLocation &Loc, uint64_t Value, bool Negative = false);

        bool isNegative() const;

        bool isPositive() const;

        uint64_t getValue() const;

        std::string str() const override;
    };

    /**
     * Used for Floating Point Numbers
     */
    class ASTFloatingValue : public ASTSingleValue {

        std::string Value;

    public:
        ASTFloatingValue(const SourceLocation &Loc, std::string &Val);

        std::string getValue() const;

        std::string str() const override;
    };

    /**
     * Used for Arrays
     */
    class ASTArrayValue : public ASTValue {

        friend class SemaBuilder;

        std::vector<ASTValue *> Values;

    public:
        ASTArrayValue(const SourceLocation &Loc);

        const std::vector<ASTValue *> &getValues() const;

        uint64_t size() const;

        bool empty() const;

        std::string str() const override;
    };

    class ASTNullValue : public ASTValue {

    public:
        ASTNullValue(const SourceLocation &Loc);

        std::string str() const override;
    };
}

#endif //FLY_ASTVALUE_H
