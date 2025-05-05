//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SymFunction.h - Symbolic Table of Function
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SYM_FUNCTIONBASE_H
#define FLY_SYM_FUNCTIONBASE_H

#include <string>
#include <llvm/ADT/SmallVector.h>

#include "SymType.h"


namespace fly {

    class ASTFunction;
    class SymVar;
    class SymErrorHandler;
    class SymParam;

class ASTVar;
    class CodeGenFunctionBase;

    enum class SymFunctionKind {
        FUNCTION,
        METHOD,
    };

    class SymFunctionBase {

        friend class SymBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        const std::string MangledName;

        llvm::SmallVector<SymParam *, 8> Params;

        SymType *ReturnType;

        ASTFunction *AST;

        SymFunctionKind Kind;

        llvm::SmallVector<SymVar *, 8> LocalVars;

        SymErrorHandler *ErrorHandler;

    protected:

        explicit SymFunctionBase(ASTFunction *AST, SymFunctionKind Kind, std::string MangledName);

    public:

        std::string getMangledName() const;

        llvm::SmallVector<SymParam *, 8> &getParams();

        SymType *getReturnType();

        ASTFunction *getAST();

        SymFunctionKind getKind() const;

        llvm::SmallVector<SymVar *, 8> getLocalVars();

        SymErrorHandler *getErrorHandler() const;

        static std::string MangleFunction(llvm::StringRef Name, const llvm::SmallVector<SymType *, 8> &Params);

        virtual CodeGenFunctionBase *getCodeGen() const = 0;

    };

}  // end namespace fly

#endif // FLY_SYM_FUNCTIONBASE_H