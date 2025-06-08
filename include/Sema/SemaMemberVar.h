//===-------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaMemberVar.h - Sybolic Table for Member Var
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_MEMBERVAR_H
#define FLY_SEMA_MEMBERVAR_H

#include "Sema/SemaVar.h"
#include "CodeGen/CodeGenVar.h"

namespace fly {

    class ASTVar;
    class CodeGenVar;

    class SemaMemberVar : public SemaVar {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        CodeGenVar *CodeGen = nullptr;

        uint64_t Index;

        explicit SemaMemberVar(ASTVar *AST, SemaResult *Parent);

    public:

        uint64_t getIndex() const;

        CodeGenVar *getCodeGen() const override;

        void setCodeGen(CodeGenVarBase *CodeGen) override;

    };

}

#endif //FLY_SEMA_MEMBERVAR_H
