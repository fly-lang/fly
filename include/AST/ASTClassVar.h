//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTField.h - Field of a Struct
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTCLASSVAR_H
#define FLY_ASTCLASSVAR_H

#include "ASTVar.h"

namespace fly {

    class ASTClass;
    class ASTClassScopes;
    class ASTType;
    class ASTValue;
    class ASTVar;

    class ASTClassVar : public ASTVar {

        friend class SemaBuilder;
        friend class SemaResolver;

        const SourceLocation &Loc;

        const ASTClass *Class = nullptr;

        std::string Comment;

        ASTClassScopes *Scopes = nullptr;

        ASTExpr *Expr = nullptr;

        CodeGenVar *CodeGen = nullptr;

        ASTClassVar(const SourceLocation &Loc, ASTClass *Class, ASTClassScopes *Scopes, ASTType *Type,
                    std::string &Name);

    public:

        const SourceLocation &getLocation() const;

        const ASTClass *getClass() const;

        const std::string &getComment() const;

        ASTClassScopes *getScopes() const;

        ASTExpr *getExpr() const override;

        CodeGenVar *getCodeGen() const override;

        virtual std::string str() const;
    };
}

#endif //FLY_ASTCLASSVAR_H
