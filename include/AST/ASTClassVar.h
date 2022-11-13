//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTClass.h - The Attribute in a Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTCLASSVAR_H
#define FLY_ASTCLASSVAR_H

#include "ASTVar.h"
#include "CodeGen/CodeGenClassVar.h"

namespace fly {

    class ASTClass;
    class ASTClassScopes;
    class ASTType;
    class ASTValue;
    class ASTVar;
    class CodeGenClassVar;

    class ASTClassVar : public ASTVar {

        friend class SemaBuilder;
        friend class SemaResolver;

        const SourceLocation &Loc;

        ASTVarKind VarKind;

        ASTType *Type = nullptr;

        llvm::StringRef Name;

        const ASTClass *Class = nullptr;

        std::string Comment;

        ASTClassScopes *Scopes = nullptr;

        ASTExpr *Expr = nullptr;

        CodeGenClassVar *CodeGen = nullptr;

        ASTClassVar(const SourceLocation &Loc, ASTClass *Class, ASTClassScopes *Scopes, ASTType *Type,
                    llvm::StringRef Name);

    public:

        const SourceLocation &getLocation() const;

        ASTVarKind getVarKind() override;

        ASTType *getType() const override;

        llvm::StringRef getName() const override;

        const ASTClass *getClass() const;

        const std::string &getComment() const;

        ASTClassScopes *getScopes() const;

        ASTExpr *getExpr() const override;

        CodeGenClassVar *getCodeGen() const;

        void setCodeGen(CodeGenClassVar *CGV);

        std::string str() const;
    };
}

#endif //FLY_ASTCLASSVAR_H
