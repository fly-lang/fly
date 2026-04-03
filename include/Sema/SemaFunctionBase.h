//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaFunction.h - Symbolic Table of Function
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_FUNCTIONBASE_H
#define FLY_SEMA_FUNCTIONBASE_H

#include "Sema/SemaNode.h"
#include <string>
#include <llvm/ADT/SmallVector.h>

#include "SemaType.h"


namespace fly {

	class SymbolTable;
    class ASTFunction;
    class SemaVar;
    class SemaError;
    class SemaParam;
    class SemaLocalVar;
    class SemaBlockStmt;
    class CodeGenFunctionBase;

    class SemaFunctionBase : public SemaNode {

        friend class SemaBuilder;

    	SymbolTable *Scope;

        llvm::SmallVector<SemaParam *, 8> Params;

        SemaType *ReturnType;

        ASTFunction &AST;

        llvm::SmallVector<SemaLocalVar *, 8> LocalVars;

    	SemaError *ErrorHandler;

    	bool Fallible;

        SemaBlockStmt *Body = nullptr;

    protected:

        explicit SemaFunctionBase(ASTFunction &AST, SemaKind Kind, SymbolTable *Symbols);

    public:

        ~SemaFunctionBase() override;

    	SymbolTable* getSymbols() const;

    	llvm::StringRef getName() const;

    	SemaType *getReturnType();

        llvm::SmallVector<SemaParam *, 8> &getParams();

        void addParam(SemaParam *Param);

        ASTFunction &getAST();

        llvm::SmallVector<SemaLocalVar *, 8> getLocalVars();

        void addLocalVar(SemaLocalVar *LocalVar);

        SemaError *getErrorHandler() const;

    	bool isFallible() const;

    	void setFallible(bool Fallible);

        SemaBlockStmt *getBody() const;

        void setBody(SemaBlockStmt *Body);

        virtual CodeGenFunctionBase *getCodeGen() const = 0;

    };

}  // end namespace fly

#endif // FLY_SEMA_FUNCTIONBASE_H