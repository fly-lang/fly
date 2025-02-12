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

#include "ASTRef.h"

namespace fly {

    class ASTArg;
    class ASTVar;
    class SymFunctionBase;

    enum class ASTCallKind {
        CALL_FUNCTION,
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
        friend class SemaResolver;
        friend class SemaValidator;

        ASTVar *ErrorHandler = nullptr;

        llvm::SmallVector<ASTArg *, 8> Args;

        SymFunctionBase **Function = nullptr;

        ASTCallKind CallKind = ASTCallKind::CALL_FUNCTION;

        ASTCall(const SourceLocation &Loc, llvm::StringRef Name);

    public:

        const ASTVar *getErrorHandler() const;

        llvm::SmallVector<ASTArg *, 8> getArgs() const;

        SymFunctionBase *getFunction() const;

        ASTCallKind getCallKind() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_CALL_H
