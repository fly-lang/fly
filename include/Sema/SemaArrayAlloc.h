//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaArrayAlloc.h - reference-counted array buffer tracking
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_ARRAY_ALLOC_H
#define FLY_SEMA_ARRAY_ALLOC_H

#include "Sema/SemaAlloc.h"

namespace fly {

    class SemaVar;

    /**
     * Tracks an array variable whose buffer is reference counted, so CodeGen can
     * emit a release at scope exit.
     *
     * Holds the SemaVar rather than the allocating call — that is what gives
     * CodeGen both the slot address (to load the current data pointer, which
     * reassignment may have changed) and the variable's TYPE, which drives the
     * recursive release of array-typed elements.
     */
    class SemaArrayAlloc : public SemaAlloc {

        SemaVar *Var;

    public:

        explicit SemaArrayAlloc(SemaVar *Var);

        ~SemaArrayAlloc() override = default;

        SemaVar *getVar() const;

        std::string str() const override;
    };

}

#endif //FLY_SEMA_ARRAY_ALLOC_H
