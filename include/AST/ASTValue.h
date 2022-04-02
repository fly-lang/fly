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

        ASTType *Ty;

    public:
        ASTValue(const SourceLocation &Loc, ASTType *Ty);

        const SourceLocation &getLocation() const;

        ASTType *getType() const;

        virtual bool empty() const = 0;

        virtual std::string str() const = 0;
    };

    /**
     * Used for Numbers, Bool
     */
    class ASTSingleValue : public ASTValue {

        std::string Str;

    public:
        ASTSingleValue(const SourceLocation &Loc, ASTType *Ty);
        ASTSingleValue(const SourceLocation &Loc, ASTType *Ty, std::string Str);

        bool empty() const override;

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

        unsigned int size() const ;

        bool empty() const override;

        std::string str() const override;
    };
}

#endif //FLY_ASTVALUE_H
