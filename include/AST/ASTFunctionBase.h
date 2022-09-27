//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTFunc.h - Function declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_FUNCTIONBASE_H
#define FLY_FUNCTIONBASE_H

#include "ASTExprStmt.h"

#include <vector>

namespace fly {

    class ASTGroupExpr;
    class ASTParams;
    class ASTExpr;
    class ASTType;
    class ASTVarRef;
    class ASTLocalVar;
    class ASTBlock;
    class ASTFunctionCall;
    class ASTGlobalVar;
    class CodeGenFunction;
    class CodeGenVar;
    class CodeGenLocalVar;
    class CodeGenCall;

    class ASTFunctionBase {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class FunctionParser;
        friend class ASTParam;

        // Function return type
        ASTType *Type = nullptr;

        // Function Name
        const std::string Name;

        // Header contains parameters
        ASTParams *Params = nullptr;

        // Body is the main BlockStmt
        ASTBlock *Body = nullptr;

        // Contains all vars declared in this Block
        std::vector<ASTLocalVar *> LocalVars;

        // Populated during codegen phase
        CodeGenFunction *CodeGen = nullptr;

    protected:

        ASTFunctionBase(ASTType *ReturnType, const std::string Name);

    public:

        ASTType *getType() const;

        const std::string getName() const;

        const ASTParams *getParams() const;

        const ASTBlock *getBody() const;

        const std::vector<ASTLocalVar *> &getLocalVars() const;

        CodeGenFunction *getCodeGen() const;

        void setCodeGen(CodeGenFunction *CGF);

        bool isVarArg();

        virtual std::string str() const;
    };

    /**
     * The Return Declaration into a Function
     * Ex.
     *   return true
     */
    class ASTReturn : public ASTExprStmt {

        friend class SemaBuilder;

        ASTReturn(ASTBlock *Parent, const SourceLocation &Loc);

    public:

        std::string str() const override;
    };
}

#endif //FLY_FUNCTIONBASE_H
