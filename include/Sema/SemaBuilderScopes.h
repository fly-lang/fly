//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaBuilderScope.h - Sema Builder Scope
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_BUILDERSCOPE_H
#define FLY_SEMA_BUILDERSCOPE_H

#include "llvm/ADT/SmallVector.h"

namespace fly {

    class ASTScope;
    class SourceLocation;

    enum class ASTVisibilityKind;

    class SemaBuilderScopes {

        llvm::SmallVector<ASTScope *, 8> Scopes;

    public:

        static SemaBuilderScopes *Create();

        SemaBuilderScopes *addVisibility(const SourceLocation &Loc, ASTVisibilityKind VisibilityKind);

        SemaBuilderScopes *addConstant(const SourceLocation &Loc, bool Constant);

        SemaBuilderScopes *addStatic(const SourceLocation &Loc, bool Static);

        llvm::SmallVector<ASTScope *, 8> getScopes() const;
    };
}

#endif //FLY_SEMA_BUILDERSCOPE_H
