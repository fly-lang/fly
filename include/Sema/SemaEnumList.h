//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaEnumList.h - enum list semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_ENUM_LIST_H
#define FLY_SEMA_ENUM_LIST_H

#include "SemaExpr.h"

namespace fly {

    class SemaEnumType;
    class SemaEnumEntry;

    /**
     * SemaEnumList - Represents a call to the built-in list() function on an enum type.
     * Returns an array of all enum entries as compile-time constants.
     *
     * Example: TestEnum.list() returns [TestEnum.A, TestEnum.B, TestEnum.C]
     */
    class SemaEnumList : public SemaExpr {

        friend class SemaBuilder;
        friend class Resolver;

        SemaEnumType *EnumType;

        explicit SemaEnumList(SemaEnumType *EnumType, SemaType *ArrayType);

    public:

        ~SemaEnumList() override = default;

        SemaEnumType *getEnumType() const;

        void accept(SemaVisitor& Visitor) override;
    };

}  // end namespace fly

#endif // FLY_SEMA_ENUM_LIST_H

