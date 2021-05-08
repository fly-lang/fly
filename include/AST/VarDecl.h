//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/VarDecl.h - Variable declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_VARDECL_H
#define FLY_VARDECL_H

#include "Decl.h"
#include "Refer.h"
#include "TypeBase.h"
#include "Expr.h"
#include "OperatorExpr.h" //TODO remove
#include "Basic/TokenKinds.h"

namespace fly {

    /**
     * Var Declaration
     */
    class VarDecl : public Decl {

        friend class Parser;
        friend class GlobalVarParser;

        const DeclKind Kind = DeclKind::D_VAR;
        TypeBase *Type;
        const StringRef Name;
        bool Constant = false;
        GroupExpr *Expression = NULL;

    public:
        VarDecl(const SourceLocation &Loc, TypeBase *Type, const StringRef Name);

        DeclKind getKind() const;

        bool isConstant() const;

        TypeBase* getType() const;

        const llvm::StringRef &getName() const;

        GroupExpr *getExpr() const;

        ~VarDecl();
    };

    /**
     * Reference to Var Declaration
     */
    class VarRef : public Refer {

        friend class Parser;

        const StringRef Name;
        VarDecl *Var = NULL;

    public:
        VarRef(const SourceLocation &Loc, const StringRef &Name);
        VarRef(const SourceLocation &Loc, VarDecl *D);

        const StringRef &getName() const;

        VarDecl *getDecl() const override;

    };

    class VarRefDecl : public VarRef, public Decl {

        friend class Parser;

        GroupExpr *Expr;

    public:
        VarRefDecl(const SourceLocation &Loc, const StringRef &Name);
        VarRefDecl(const SourceLocation &Loc, VarDecl *D);

        DeclKind getKind() const override;

        GroupExpr *getExpr() const;
    };
}

#endif //FLY_VARDECL_H
