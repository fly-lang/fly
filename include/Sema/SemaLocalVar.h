//===-------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaLocalVar.h - Sybolic Table for Local ASTVar
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_LOCALVAR_H
#define FLY_SEMA_LOCALVAR_H

#include "Sema/SemaVar.h"
#include "CodeGen/CodeGenVar.h"

namespace fly {

    class ASTVar;
    class CodeGenVar;

    class SemaLocalVar : public SemaVar {

        friend class SemaBuilder;
        friend class Resolver;
        friend class SemaValidator;

        CodeGenVar *CodeGen;

        explicit SemaLocalVar(ASTVar *AST);

    public:

        CodeGenVar *getCodeGen() const override;

        void setCodeGen(CodeGenVarBase *CodeGen) override;

    };

}

#endif //FLY_SEMA_LOCALVAR_H
