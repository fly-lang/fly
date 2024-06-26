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

#include "ASTStmt.h"
#include "ASTBase.h"

#include <vector>

namespace fly {

    class ASTGroupExpr;
    class ASTParams;
    class ASTExpr;
    class ASTType;
    class ASTVarRef;
    class ASTLocalVar;
    class ASTBlock;
    class ASTCall;
    class ASTGlobalVar;
    class ASTVar;
    class ASTParam;
    class ASTScopes;
    class CodeGenFunction;
    class CodeGenFunctionBase;
    class CodeGenVarBase;


    enum class ASTFunctionKind {
        FUNCTION,
        CLASS_FUNCTION
    };

    class ASTFunctionBase : public ASTBase {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class FunctionParser;
        friend class ClassParser;
        friend class ASTParam;

        ASTFunctionKind Kind;

        // Function return type
        ASTType *ReturnType = nullptr;

        // Function Name
        llvm::StringRef Name;

        ASTScopes * Scopes;

        // Header contains parameters
        ASTParams *Params = nullptr;

        // Body is the main BlockStmt
        ASTBlock *Body = nullptr;

    protected:

        ASTFunctionBase(const SourceLocation &Loc, ASTFunctionKind Kind, ASTType *ReturnType, llvm::StringRef Name,
                        ASTScopes *Scopes);

    public:

        ASTFunctionKind getKind();

        ASTType *getType() const;

        llvm::StringRef getName() const;

        ASTScopes *getScopes() const;

        void addParam(ASTParam *Param);

        void setEllipsis(ASTParam *Param);

        const ASTParams *getParams() const;

        const ASTBlock *getBody() const;

        virtual CodeGenFunctionBase *getCodeGen() const = 0;

        bool isVarArg();

        std::string str() const override;
    };

    /**
     * The Return Declaration into a Function
     * Ex.
     *   return true
     */
    class ASTReturnStmt : public ASTStmt {

        friend class SemaBuilder;

        ASTExpr *Expr = nullptr;

        ASTBlock *Block = nullptr;

        ASTReturnStmt(ASTBlock *Parent, const SourceLocation &Loc);

    public:

        ASTExpr *getExpr() const;

        ASTBlock *getBlock() const;

        std::string str() const;
    };
}

#endif //FLY_FUNCTIONBASE_H
