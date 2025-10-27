//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTCall.h - AST Call header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_CALL_H
#define FLY_AST_CALL_H

#include <Sema/SemaErrorHandler.h>

#include "ASTRef.h"

namespace fly {

    class ASTArg;
    class ASTVar;
    class SemaCall;

    enum class ASTCallKind {
        CALL_DIRECT,
        CALL_NEW,
        CALL_NEW_UNIQUE,
        CALL_NEW_SHARED,
        CALL_NEW_WEAK
    };

    /**
     * A Reference to a Function in a Declaration
     * Ex.
     *  int a = sqrt(4)
     */
    class ASTCall : public ASTRef {

        friend class ASTBuilder;
        friend class SemaBuilder;
        friend class Resolver;
        friend class SemaValidator;

        const ASTCallKind CallKind;

        llvm::SmallVector<ASTArg *, 8> Args;

        SemaCall *Sema = nullptr;

        ASTCall(const SourceLocation &Loc, llvm::StringRef Name, ASTCallKind CallKind);

    public:

        llvm::SmallVector<ASTArg *, 8> getArgs() const;

        SemaCall *getSema() const;

        ASTCallKind getCallKind() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_CALL_H
