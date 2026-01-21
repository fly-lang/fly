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

    class ASTFunction;
    class SemaVar;
    class SemaErrorHandler;
    class SemaParam;
    class SemaLocalVar;
    class CodeGenFunctionBase;

    class SemaFunctionBase : public SemaNode {

        friend class SemaBuilder;

        llvm::SmallVector<SemaParam *, 8> Params;

        SemaType *ReturnType;

    	SemaValue *DefaultReturnValue;

        ASTFunction &AST;

        llvm::SmallVector<SemaLocalVar *, 8> LocalVars;

        SemaErrorHandler *ErrorHandler;

    protected:

        explicit SemaFunctionBase(ASTFunction &AST, SemaKind Kind);

    public:

        ~SemaFunctionBase() override;

    	llvm::StringRef getName() const;

    	SemaType *getReturnType();

        void setReturnType(SemaType *RetType);

    	SemaValue *getDefaultReturnValue() const;

    	void setDefaultReturnValue(SemaValue *Value);

        llvm::SmallVector<SemaParam *, 8> &getParams();

        void addParam(SemaParam *Param);

        ASTFunction &getAST();

        llvm::SmallVector<SemaLocalVar *, 8> getLocalVars();

        void addLocalVar(SemaLocalVar *LocalVar);

        SemaErrorHandler *getErrorHandler() const;

        virtual CodeGenFunctionBase *getCodeGen() const = 0;

    };

}  // end namespace fly

#endif // FLY_SEMA_FUNCTIONBASE_H