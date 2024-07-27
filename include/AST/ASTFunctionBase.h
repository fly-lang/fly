//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTFunctionBase.h - AST Function Base header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_FUNCTIONBASE_H
#define FLY_AST_FUNCTIONBASE_H

#include "ASTStmt.h"
#include "ASTBase.h"

namespace fly {

    class ASTGroupExpr;
    class ASTExpr;
    class ASTType;
    class ASTVarRef;
    class ASTLocalVar;
    class ASTBlockStmt;
    class ASTCall;
    class ASTGlobalVar;
    class ASTVar;
    class ASTParam;
    class ASTScope;
    class CodeGenFunction;
    class CodeGenFunctionBase;
    class CodeGenVarBase;

    enum class ASTFunctionKind {
        FUNCTION,
        CLASS_METHOD
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

        llvm::SmallVector<ASTScope *, 8> Scopes;

        llvm::SmallVector<ASTParam *, 8> Params;

        ASTParam* Ellipsis = nullptr;

        llvm::SmallVector<ASTLocalVar *, 8> LocalVars;

        // Body is the main BlockStmt
        ASTBlockStmt *Body = nullptr;

        ASTParam *ErrorHandler = nullptr;

    protected:

        ASTFunctionBase(const SourceLocation &Loc, ASTFunctionKind Kind, ASTType *ReturnType, llvm::SmallVector<ASTScope *, 8> &Scopes);

    public:

        ASTFunctionKind getKind();

        ASTType *getReturnType() const;

        llvm::SmallVector<ASTScope *, 8> getScopes() const;

        llvm::SmallVector<ASTParam *, 8> getParams() const;

        ASTParam *getEllipsis() const;

        void setEllipsis(ASTParam *Param);

        llvm::SmallVector<ASTLocalVar *, 8> getLocalVars() const;

        ASTBlockStmt *getBody() const;

        ASTParam *getErrorHandler();

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
        friend class SemaResolver;

        ASTExpr *Expr = nullptr;

        ASTReturnStmt(const SourceLocation &Loc);

    public:

        ASTExpr *getExpr() const;

        std::string str() const override;
    };
}

#endif //FLY_AST_FUNCTIONBASE_H
