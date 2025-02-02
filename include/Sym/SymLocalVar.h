//===-------------------------------------------------------------------------------------------------------------===//
// include/Sym/SymLocalVar.h - Sybolic Table for Local ASTVar
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SYM_LOCALVAR_H
#define FLY_SYM_LOCALVAR_H

#include "Sym/SymVar.h"
#include "CodeGen/CodeGenVar.h"

namespace fly {

    class ASTVar;
class CodeGenVar;

    class SymLocalVar : public SymVar {

        friend class SymBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        CodeGenVar *CodeGen;

        explicit SymLocalVar(ASTVar *AST);

    public:

        CodeGenVar *getCodeGen() const override;

    };

}

#endif //FLY_SYM_LOCALVAR_H
