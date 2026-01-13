//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTEnumValue.h - AST Enum Value header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_ENUMVALUE_H
#define FLY_AST_ENUMVALUE_H

#include "ASTValue.h"

#include <Sema/SemaEnumValue.h>

namespace fly {

    class SourceLocation;
    class ASTEnum;

    /**
     * Represents a constant enum value (e.g., Color.RED)
     */
    class ASTEnumValue : public ASTValue {

        friend class ASTBuilder;

        ASTEnum *Enum;

        llvm::StringRef Name;

        uint32_t Index = 0;

        SemaEnumValue *Sema;

    protected:

        ASTEnumValue(const SourceLocation &Loc, ASTEnum *Enum, llvm::StringRef Name);

    public:

        void accept(ASTVisitor& Visitor) override;

        ASTEnum *getEnum() const;

        llvm::StringRef getName() const;

        uint32_t getIndex() const;

        void setIndex(uint32_t Index);

        SemaEnumValue *getSema() const override;

        void setSema(SemaEnumValue *Sema);

        std::string str() const override;
    };
}

#endif //FLY_AST_ENUMVALUE_H
