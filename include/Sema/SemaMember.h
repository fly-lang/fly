//===-------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaMember.h - member access semantic analysis
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

    class SemaMember : public SemaExpr {

        friend class SemaBuilder;
        friend class Resolver;
        friend class SemaValidator;

    	ASTMember &AST;

        SemaExpr *Ref;

    	CodeGenExpr *CodeGen = nullptr;

        explicit SemaMember(ASTMember &AST, SemaExpr *Ref, SemaExpr *Parent);

    public:

        ~SemaMember() override = default;

    	ASTMember &getAST() const;

        SemaExpr *getRef() const;

        CodeGenExpr *getCodeGen() const;

        void setCodeGen(CodeGenExpr *CodeGen);

        std::string str() const override;

        void accept(SemaVisitor& Visitor) override;

    };

}

#endif //FLY_SEMA_MEMBERVAR_H
