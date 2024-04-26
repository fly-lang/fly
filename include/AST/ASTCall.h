//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTCall.h - Call declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_FUNCTION_CALL_H
#define FLY_FUNCTION_CALL_H

#include "ASTExpr.h"
#include "AST/ASTIdentifier.h"

#include "llvm/ADT/SmallVector.h"

#include <vector>

namespace fly {

    class ASTType;
    class ASTFunctionBase;
    class ASTParam;
    class ASTArg;
    class ASTCallExpr;
    class ASTVar;
    class ASTError;

    enum class ASTCallKind {
        CALL_FUNCTION,
        CALL_CONSTRUCTOR
    };

    enum class ASTMemoryKind {
        CALL_MANAGED,
        CALL_UNIQUE,
        CALL_SHARED,
        CALL_WEAK
    };

    /**
     * A Reference to a Function in a Declaration
     * Ex.
     *  int a = sqrt(4)
     */
    class ASTCall : public ASTIdentifier {

        friend class SemaBuilder;
        friend class SemaResolver;

        ASTError *ErrorHandler = nullptr;

        std::vector<ASTArg *> Args;

        ASTFunctionBase *Def = nullptr;

        ASTCallKind CallKind = ASTCallKind::CALL_FUNCTION;

        ASTMemoryKind MemoryKind = ASTMemoryKind::CALL_MANAGED;

        ASTCall(const SourceLocation &Loc, llvm::StringRef Name);

        ASTCall(ASTFunctionBase *Function);

    public:

        const ASTError *getErrorHandler() const;

        const std::vector<ASTArg *> getArgs() const;

        ASTFunctionBase *getDef() const;

        ASTCallKind getCallKind() const;

        ASTMemoryKind getMemoryKind() const;

        std::string str() const;
    };

    class ASTArg : public ASTBase {

        friend class SemaResolver;
        friend class SemaBuilder;

        ASTExpr *Expr;

        uint64_t Index;

        ASTParam *Def = nullptr;

        ASTCall *Call = nullptr;

        ASTArg(ASTCall *Call, ASTExpr *Expr);

    public:

        ASTExpr *getExpr() const;

        uint64_t getIndex() const;

        ASTParam *getDef() const;

        ASTCall *getCall() const;

        std::string str() const;

    };
}

#endif //FLY_FUNCTION_CALL_H
