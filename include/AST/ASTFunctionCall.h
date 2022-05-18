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

#include "ASTStmt.h"
#include <vector>

namespace fly {

    class ASTType;
    class CodeGenCall;

    /**
     * A Reference to a Function in a Declaration
     * Ex.
     *  int a = sqrt(4)
     */
    class ASTFunctionCall {

        friend class SemaBuilder;
        friend class SemaResolver;

        const SourceLocation Loc;
        std::string NameSpace;
        const std::string Name;
        std::vector<ASTExpr *> Args;
        ASTFunction *Def = nullptr;
        CodeGenCall *CGC = nullptr;

    public:
        ASTFunctionCall(const SourceLocation &Loc, const std::string &NameSpace, const std::string &Name);

        const SourceLocation &getLocation() const;

        const std::string &getNameSpace() const;

        void setNameSpace(const std::string &NameSpace);

        const std::string &getName() const;

        const std::vector<ASTExpr *> getArgs() const;

        ASTFunction *getDef() const;

        CodeGenCall *getCodeGen() const;

        void setCodeGen(CodeGenCall *CGC);

        std::string str() const;
    };
}

#endif //FLY_FUNCTION_CALL_H
