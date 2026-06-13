//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaEnumAccessor.h - enum accessor semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_ENUM_ACCESSOR_H
#define FLY_SEMA_ENUM_ACCESSOR_H

#include "SemaExpr.h"

namespace fly {

    class SemaEnumType;
    class SemaEnumEntry;
    class SemaVar;

    /**
     * SemaEnumAccessor - Represents the built-in property accessors on an enum
     * entry or an enum-typed value:
     *
     *   Color.RED.name   → string  "RED"
     *   Color.RED.value  → int     (the entry index)
     *   c.name / c.value where `c` is an enum-typed variable
     *
     * Exactly one of {Entry, Var} is set:
     *   - Entry != null : literal entry access — folds to a compile-time constant.
     *   - Var   != null : access on an enum-typed variable — `.value` is the
     *                     variable's i32, `.name` is a load from the enum's
     *                     runtime names table indexed by that i32.
     */
    class SemaEnumAccessor : public SemaExpr {

        friend class SemaBuilder;
        friend class Resolver;

        SemaEnumType *EnumType;

        SemaEnumEntry *Entry;   // non-null for a literal entry; null for a variable

        SemaVar *Var;           // non-null for a variable; null for a literal entry

        bool IsName;            // true = .name (string), false = .value (int)

        explicit SemaEnumAccessor(SemaEnumType *EnumType, SemaEnumEntry *Entry,
                                  SemaVar *Var, bool IsName, SemaType *Type);

    public:

        ~SemaEnumAccessor() override = default;

        SemaEnumType *getEnumType() const;

        SemaEnumEntry *getEntry() const;

        SemaVar *getVar() const;

        bool isName() const;

        std::string str() const override;

        void accept(SemaVisitor& Visitor) override;
    };

}  // end namespace fly

#endif // FLY_SEMA_ENUM_ACCESSOR_H
