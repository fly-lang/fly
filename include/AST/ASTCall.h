//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTFunc.h - Function declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_FUNCTION_CALL_H
#define FLY_FUNCTION_CALL_H

#include "ASTExpr.h"
#include "Basic/Debuggable.h"

#include "llvm/ADT/SmallVector.h"

#include <vector>

namespace fly {

    class ASTType;
    class CodeGenCall;
    class ASTFunctionBase;
    class ASTParam;
    class ASTArg;
    class ASTCallExpr;
    class ASTVar;
    class ASTIdentifier;
    class ASTIdentifier;

    /**
     * A Reference to a Function in a Declaration
     * Ex.
     *  int a = sqrt(4)
     */
    class ASTCall : public Debuggable {

        friend class SemaBuilder;
        friend class SemaResolver;

        ASTIdentifier *Identifier = nullptr;

        std::vector<ASTArg *> Args;

        ASTFunctionBase *Def = nullptr;

        CodeGenCall *CGC = nullptr;

        ASTVar *Instance = nullptr;

        bool New = false;

        ASTCall(ASTIdentifier *Identifier);

        ASTCall(ASTFunctionBase *Function);

    public:

        SourceLocation getLocation() const;

        llvm::StringRef getName() const;

        ASTIdentifier *getIdentifier() const;

        const std::vector<ASTArg *> getArgs() const;

        ASTFunctionBase *getDef() const;

        CodeGenCall *getCodeGen() const;

        ASTVar *getInstance() const;

        bool isNew() const;

        std::string str() const;
    };

    class ASTArg : public Debuggable {

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
