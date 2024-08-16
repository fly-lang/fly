//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaClassSymbols.h - SemaClassSymbols
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_CLASSSYMBOLS_H
#define FLY_SEMA_CLASSSYMBOLS_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"

#include <map>

namespace fly {

    class Sema;
    class ASTIdentity;
    class ASTClassAttribute;
    class ASTClassMethod;

    class SemaIdentitySymbols {

        friend class Sema;
        friend class SemaResolver;

        ASTIdentity *Identity;

        // Class Attributes
        llvm::StringMap<ASTClassAttribute *> Attributes;

        // Class Functions
        llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTClassMethod *, 4>>> Methods;

        explicit SemaIdentitySymbols(ASTIdentity *Identity);

    public:

        ASTIdentity *getIdentity();

        llvm::StringMap<ASTClassAttribute *> getAttributes();

        llvm::StringMap<std::map <uint64_t,llvm::SmallVector <ASTClassMethod *, 4>>> getMethods();
    };

}  // end namespace fly

#endif