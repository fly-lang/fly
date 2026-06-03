//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaContext.h - sema context and coordination
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_H
#define FLY_SEMA_H

#include "Resolver.h"
#include <llvm/ADT/SmallVector.h>

namespace llvm {
    class StringRef;
}

namespace fly {

    class DiagnosticsEngine;

    class SemaContext {

        DiagnosticsEngine &Diags;

        Registry *Reg;

        ASTBuilder *Builder = nullptr;

    public:

        explicit SemaContext(DiagnosticsEngine& diags);

        ~SemaContext();

        llvm::SmallVector<SemaModule *, 8> Resolve(llvm::SmallVector<ASTModule *, 8> &Modules,
                                                    bool TestMode = false);

    };

}  // end namespace fly

#endif // FLY_SEMA_H