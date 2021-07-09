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
#include "Stmt.h"

namespace fly {

    class VarDecl {

        friend class ASTNode;
        friend class Parser;
        friend class GlobalVarParser;
        friend class FunctionParser;
        friend class VarDeclStmt;

        TypeBase *Type;
        const llvm::StringRef NameSpace;
        const llvm::StringRef Name;
        bool Constant = false;
        GroupExpr *Expression = NULL;

    public:
        VarDecl(TypeBase *Type, const StringRef &Name, const StringRef &NameSpace = "");

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

        const SourceLocation Loc;
        const llvm::StringRef NameSpace;
        const llvm::StringRef Name;

        unsigned long Order;

        VarDecl *Decl = nullptr;

    public:
        VarRef(const SourceLocation &Loc, const llvm::StringRef &Name, const llvm::StringRef &NameSpace = "");

        const SourceLocation &getLocation() const;

        const StringRef &getNameSpace() const;

        const llvm::StringRef &getName() const;

        unsigned long getOrder() const;

        void setOrder(unsigned long order);

        VarDecl *getDecl() const;

        void setDecl(VarDecl *decl);
    };

    /**
     * Declaration of a reference to a Var
     * Ex.
     *  a = 1
     */
    class VarStmt : public VarRef, public Stmt {

        GroupExpr *Expr;

    public:
        VarStmt(const SourceLocation &Loc, BlockStmt *Block, const llvm::StringRef &Name,
                const llvm::StringRef &NameSpace = "");

        StmtKind getKind() const override;

        GroupExpr *getExpr() const;

        void setExpr(GroupExpr *E);
    };
}

#endif //FLY_VARDECL_H
