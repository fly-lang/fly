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
    class ASTVar;
    class CodeGenFunctionBase;

    class SemaFunctionBase : public SemaNode {

        friend class SemaBuilder;

        const std::string MangledName;

        llvm::SmallVector<SemaParam *, 8> Params;

        SemaType *ReturnType;

        ASTFunction &AST;

        llvm::SmallVector<SemaVar *, 8> LocalVars;

        SemaErrorHandler *ErrorHandler;

    protected:

        explicit SemaFunctionBase(ASTFunction &AST, SemaKind Kind, std::string MangledName);

        std::string MangleFunction(ASTFunction &AST);

    public:

        std::string getMangledName() const;

        void setReturnType(SemaType *RetType);

        llvm::SmallVector<SemaParam *, 8> &getParams();

        void addParam(SemaParam *Param);

        SemaType *getReturnType();

        ASTFunction &getAST();

        llvm::SmallVector<SemaVar *, 8> getLocalVars();

        void addLocalVar(SemaVar *LocalVar);

        SemaErrorHandler *getErrorHandler() const;

        virtual CodeGenFunctionBase *getCodeGen() const = 0;

    };

}  // end namespace fly

#endif // FLY_SEMA_FUNCTIONBASE_H