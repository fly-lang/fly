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
#include "CodeGen/CodeGenVar.h"

namespace fly {

    class ASTClass;
    class ASTScopes;
    class ASTType;
    class ASTValue;
    class ASTVar;

    class ASTClassVar : public ASTVar {

        friend class SemaBuilder;
        friend class SemaResolver;

        const SourceLocation &Loc;

        ASTVarKind VarKind;

        ASTType *Type = nullptr;

        llvm::StringRef Name;

        ASTClass *Class = nullptr;

        llvm::StringRef Comment;

        ASTScopes *Scopes = nullptr;

        ASTExpr *Expr = nullptr;

        CodeGenVarBase *CodeGen = nullptr;

        ASTClassVar(const SourceLocation &Loc, ASTClass *Class, ASTScopes *Scopes, ASTType *Type,
                    llvm::StringRef Name);

    public:

        const SourceLocation &getLocation() const;

        ASTVarKind getVarKind() override;

        ASTType *getType() const override;

        llvm::StringRef getName() const override;

        ASTClass *getClass() const;

        llvm::StringRef getComment() const;

        ASTScopes *getScopes() const;

        ASTExpr *getExpr() const override;

        void setExpr(ASTExpr *expr);

        CodeGenVarBase *getCodeGen() const;

        void setCodeGen(CodeGenVarBase *CGV);

        std::string print() const;

        std::string str() const;
    };
}

#endif //FLY_ASTCLASSVAR_H
