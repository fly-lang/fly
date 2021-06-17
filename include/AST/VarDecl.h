//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/VarDecl.h - Var declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_VARDECL_H
#define FLY_VARDECL_H

#include "TypeBase.h"
#include "Expr.h"

namespace fly {

    class VarDecl {

        friend class ASTNode;
        friend class Parser;
        friend class GlobalVarParser;
        friend class FunctionParser;
        friend class VarDeclStmt;

        const bool Global;
        TypeBase *Type;
        const llvm::StringRef Name;
        bool Constant = false;
        GroupExpr *Expression = NULL;

    public:
        VarDecl(TypeBase *Type, const StringRef &Name, bool isGlobal = false);
        virtual ~VarDecl();

        const bool isGlobal() const;

        virtual bool isConstant() const;

        virtual TypeBase *getType() const;

        virtual const llvm::StringRef &getName() const;

        void setExpr(GroupExpr *Exp);

        GroupExpr *getExpr() const;
    };

    /**
     * Reference to Var Declaration
     * Ex.
     *  ... = a + 1
     */
    class VarRef {

        friend class Parser;

        const llvm::StringRef Name;
        VarDecl *Var = NULL;

    public:
        VarRef(const SourceLocation &Loc, const llvm::StringRef &Name);
        VarRef(const SourceLocation &Loc, VarDecl *D);

        const llvm::StringRef &getName() const;

        VarDecl *getVarDecl() const;

    };

    /**
     * Declaration of a reference to a Var
     * Ex.
     *  a = 1
     */
    class VarStmt : public VarRef, public Stmt {

        friend class Parser;

        GroupExpr *Expr;

    public:
        VarStmt(const SourceLocation &Loc, BlockStmt *CurrStmt, const llvm::StringRef &Name);
        VarStmt(const SourceLocation &Loc, BlockStmt *CurrStmt, VarDecl *D);

        StmtKind getKind() const override;

        GroupExpr *getExpr() const;

        void setExpr(GroupExpr *Exp);
    };
}

#endif //FLY_VARDECL_H
