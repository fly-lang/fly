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

        ASTFunction *AST;

        SymFunctionKind Kind;

        explicit SymFunctionBase(ASTFunction *AST, SymFunctionKind Kind);

    public:

        static std::string MangleFunction(ASTFunction *AST);

        static std::string MangleFunction(llvm::StringRef Name, const llvm::SmallVector<SymType *, 8> &Params);

        ASTFunction *getAST();

        virtual CodeGenFunctionBase *getCodeGen() const;

    };

}  // end namespace fly

#endif // FLY_SYM_FUNCTIONBASE_H