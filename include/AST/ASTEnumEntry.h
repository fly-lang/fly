//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTEnumEntry.h - AST Enum Entry header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_ENUMENTRY_H
#define FLY_AST_ENUMENTRY_H

#include "ASTExpr.h"

namespace fly {

    class SourceLocation;
    class ASTEnum;
    class Symbol;

    /**
     * Represents a constant enum entry (e.g., Color.RED)
     */
    class ASTEnumEntry : public ASTExpr {

        friend class ASTBuilder;

        ASTEnum *Enum;

        llvm::StringRef Name;

        uint32_t Index = 0;

        Symbol *Sym = nullptr;

    protected:

        ASTEnumEntry(const SourceLocation &Loc, ASTEnum *Enum, llvm::StringRef Name);

    public:

        void accept(ASTVisitor& Visitor) override;

        ASTEnum *getEnum() const;

        llvm::StringRef getName() const;

        uint32_t getIndex() const;

        void setIndex(uint32_t Index);

        Symbol *getSymbol() const;

        void setSymbol(Symbol *Sym);

        std::string str() const override;
    };
}

#endif //FLY_AST_ENUMENTRY_H
