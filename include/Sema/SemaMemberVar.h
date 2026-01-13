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

    class ASTMember;
    class CodeGenVar;

    class SemaMemberVar : public SemaVar {

        friend class SemaBuilder;
        friend class Resolver;
        friend class SemaValidator;

        SemaClassAttribute *ClassAttribute;

        CodeGenVar *CodeGen = nullptr;

        explicit SemaMemberVar(ASTVar &AST, SemaExpr &Parent, SemaClassAttribute *Attribute);

    public:

        ~SemaMemberVar() override = default;

        SemaClassAttribute *getClassAttribute() const;

        void setClassAttribute(SemaClassAttribute *ClassAttribute);

        CodeGenVar *getCodeGen() const override;

        void setCodeGen(CodeGenVarBase *CodeGen) override;

        void accept(SemaVisitor& Visitor) override;

    };

}

#endif //FLY_SEMA_MEMBERVAR_H
