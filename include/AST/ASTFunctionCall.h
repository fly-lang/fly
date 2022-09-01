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

#include "ASTExprStmt.h"
#include "llvm/ADT/SmallVector.h"
#include <vector>

namespace fly {

    class ASTType;
    class CodeGenCall;
    class ASTFunctionCall;
    class ASTParam;
    class ASTArg;

    /**
     * A Reference to a Function in a Declaration
     * Ex.
     *  int a = sqrt(4)
     */
    class ASTFunctionCall {

        friend class SemaBuilder;
        friend class SemaResolver;

        const SourceLocation Loc;

        ASTStmt *Stmt = nullptr;

        const std::string Name;

        std::string NameSpace;

        std::vector<ASTArg *> Args;

        ASTFunction *Def = nullptr;

        CodeGenCall *CGC = nullptr;

        ASTFunctionCall(const SourceLocation &Loc, const std::string NameSpace, const std::string Name);

    public:

        const SourceLocation &getLocation() const;

        const std::string getNameSpace() const;

        const std::string getName() const;

        const std::vector<ASTArg *> getArgs() const;

        ASTFunction *getDef() const;

        CodeGenCall *getCodeGen() const;

        std::string str() const;
    };

    class ASTArg : public ASTExprStmt {

        friend class SemaResolver;
        friend class SemaBuilder;

        uint64_t Index;

        ASTParam *Def = nullptr;

        ASTFunctionCall *Call = nullptr;

        ASTArg(const SourceLocation &Loc);

    public:

        StmtKind getKind() const override;

        uint64_t getIndex() const;

        ASTParam *getDef() const;

        ASTFunctionCall *getCall() const;

        std::string str() const override;

    };
}

#endif //FLY_FUNCTION_CALL_H
