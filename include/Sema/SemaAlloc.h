//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaAlloc.h - Base class for scope-managed allocations
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_ALLOC_H
#define FLY_SEMA_ALLOC_H

namespace fly {

    enum class SemaAllocKind {
        SMART,   // smart-pointer allocation (unique / shared / weak)
        STRING,  // heap-owned string buffer
    };

    /**
     * Abstract base for any allocation registered in a SemaBlockStmt that
     * requires a scope-exit cleanup. Subclasses carry the data needed by
     * CodeGen to emit the appropriate free / release sequence.
     */
    class SemaAlloc {

        SemaAllocKind Kind;

    protected:

        explicit SemaAlloc(SemaAllocKind Kind) : Kind(Kind) {}

    public:

        virtual ~SemaAlloc() = default;

        SemaAllocKind getKind() const { return Kind; }
    };

}

#endif //FLY_SEMA_ALLOC_H
