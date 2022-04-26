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

#include "ASTType.h"
#include <vector>

namespace fly {

    class ASTValue {

        const SourceLocation &Loc;

        ASTType *Type;

    public:
        ASTValue(const SourceLocation &Loc, ASTType *Type);

        const SourceLocation &getLocation() const;

        ASTType *getType() const;

        virtual std::string str() const = 0;
    };

    /**
     * Abstract Value for Integer, Floating Point and Boolean
     */
    class ASTSingleValue : public ASTValue {

    public:
        ASTSingleValue(const SourceLocation &Loc, ASTType *Ty);
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

        bool Negative; // true is positive, false is negative
        uint64_t Value;

    public:
        ASTIntegerValue(const SourceLocation &Loc, ASTType *Ty, uint64_t Value = 0, bool Negative = false);

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
        ASTFloatingValue(const SourceLocation &Loc, ASTType *Type, std::string &Val);

        std::string getValue() const;

        std::string str() const override;
    };

    /**
     * Used for Arrays
     */
    class ASTArrayValue : public ASTValue {

        std::vector<ASTValue *> Values;

    public:
        ASTArrayValue(const SourceLocation &Loc, ASTType *Type);

        void addValue(ASTValue * Value);

        const std::vector<ASTValue *> &getValues() const;

        uint64_t size() const;

        bool empty() const;

        std::string str() const override;
    };
}

#endif //FLY_ASTVALUE_H
