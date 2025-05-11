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

    enum class SemaFunctionKind {
        FUNCTION,
        METHOD,
    };

    class SemaFunctionBase {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaResolverClass;
        friend class SemaValidator;

        const std::string MangledName;

        llvm::SmallVector<SemaParam *, 8> Params;

        SemaType *ReturnType;

        ASTFunction *AST;

        SemaFunctionKind Kind;

        llvm::SmallVector<SemaVar *, 8> LocalVars;

        SemaErrorHandler *ErrorHandler;

    protected:

        explicit SemaFunctionBase(ASTFunction *AST, SemaFunctionKind Kind, std::string MangledName);

    public:

        std::string getMangledName() const;

        llvm::SmallVector<SemaParam *, 8> &getParams();

        SemaType *getReturnType();

        ASTFunction *getAST();

        SemaFunctionKind getKind() const;

        llvm::SmallVector<SemaVar *, 8> getLocalVars();

        SemaErrorHandler *getErrorHandler() const;

        static std::string MangleFunction(llvm::StringRef Name, const llvm::SmallVector<SemaType *, 8> &Params);

        virtual CodeGenFunctionBase *getCodeGen() const = 0;

    };

}  // end namespace fly

#endif // FLY_SEMA_FUNCTIONBASE_H