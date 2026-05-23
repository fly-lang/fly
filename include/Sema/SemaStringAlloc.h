//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaStringAlloc.h - heap string allocation tracking
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_STRING_ALLOC_H
#define FLY_SEMA_STRING_ALLOC_H

#include "Sema/SemaAlloc.h"

namespace fly {

    class SemaVar;

    /**
     * Tracks a single heap-owned string variable inside a scope. Holds a
     * pointer to the SemaVar so that CodeGen can load the current string struct,
     * extract the data pointer (field 0), and emit a free() at scope exit.
     */
    class SemaStringAlloc : public SemaAlloc {

        SemaVar *Var;

    public:

        explicit SemaStringAlloc(SemaVar *Var);

        ~SemaStringAlloc() override = default;

        SemaVar *getVar() const;
    };

}

#endif //FLY_SEMA_STRING_ALLOC_H
