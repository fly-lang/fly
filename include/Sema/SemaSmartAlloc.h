//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaSmartAlloc.h - smart pointer allocation tracking
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_SMART_ALLOC_H
#define FLY_SEMA_SMART_ALLOC_H

#include "Sema/SemaAlloc.h"

#include <cstdint>

namespace fly {

    class SemaCall;

    /**
     * Tracks a single smart-pointer allocation inside a scope. Holds the resolved
     * SemaCall so that CodeGen can retrieve the heap pointer via
     * getCall()->getCodeGen()->getValue() and emit cleanup (free / shared_release)
     * at scope exit.
     *
     * Each allocation — original or copy-reference — gets its own entry. The
     * Call's getValue() is always the canonical pointer for that entry: for a
     * copy (u = t) the SA wraps the original CALL_NEW_SHARED call whose return
     * value equals what u holds, so no separate variable tracking is needed.
     */
    class SemaSmartAlloc : public SemaAlloc {

        SemaCall    *Call;
        uint64_t    ReferenceCounter;

    public:

        explicit SemaSmartAlloc(SemaCall *Call);

        ~SemaSmartAlloc() override = default;

        SemaCall *getCall() const;

        bool isUnique() const;

        bool isShared() const;

        bool isWeak() const;

        uint64_t getReferenceCounter() const;

        uint64_t incrReferenceCounter();

        uint64_t decrReferenceCounter();

        std::string str() const override;
    };
}

#endif //FLY_SEMA_SMART_ALLOC_H
