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

#include "Basic/Debuggable.h"

#include "llvm/ADT/StringMap.h"

#include <string>
#include <vector>

namespace fly {

    class ASTType;
    class SourceLocation;

    enum class ASTTypeKind;

    class ASTValue : public Debuggable {

        friend class SemaBuilder;
        friend class SemaResolver;

        const SourceLocation &Location;

        const ASTTypeKind MacroKind;

    protected:

        ASTValue(const ASTTypeKind Kind, const SourceLocation &Location);

    public:

        const SourceLocation &getLocation() const;

        const ASTTypeKind &getMacroKind() const;

        const std::string printMacroType() const;

        virtual const std::string print() const = 0;

        std::string str() const;
    };

    /**
     * Used for Integer Numbers
     */
    class ASTBoolValue : public ASTValue {

        friend class SemaBuilder;

        bool Value;

        ASTBoolValue(const SourceLocation &Loc, bool Value = false);

    public:

        bool getValue() const;

        const std::string print() const;

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

        bool isPositive() const;

        uint64_t getValue() const;

        const std::string print() const;

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

        const std::string print() const;

        std::string str() const override;
    };

    /**
     * Used for Arrays
     */
    class ASTArrayValue : public ASTValue {

        friend class SemaBuilder;

        std::vector<ASTValue *> Values;

        ASTArrayValue(const SourceLocation &Loc);

    public:

        const std::vector<ASTValue *> &getValues() const;

        uint64_t size() const;

        bool empty() const;

        const std::string print() const;

        std::string str() const override;
    };

    /**
     * Used for Structs
     */
    class ASTStructValue : public ASTValue {

        friend class SemaBuilder;

        llvm::StringMap<ASTValue *> Values;

        ASTStructValue(const SourceLocation &Loc);

    public:

        const llvm::StringMap<ASTValue *> &getValues() const;

        uint64_t size() const;

        bool empty() const;

        const std::string print() const;

        std::string str() const override;
    };

    class ASTNullValue : public ASTValue {

        friend class SemaBuilder;

        ASTNullValue(const SourceLocation &Loc);

    public:

        const std::string print() const;

        std::string str() const override;
    };

    class ASTZeroValue : public ASTValue {

        friend class SemaBuilder;

        ASTZeroValue(const SourceLocation &Loc);

    public:

        const std::string print() const;

        std::string str() const override;
    };
}

#endif //FLY_ASTVALUE_H
