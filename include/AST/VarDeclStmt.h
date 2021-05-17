//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/VarDeclStmt.h - Variable declaration statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_VARDECLSTMT_H
#define FLY_VARDECLSTMT_H

#include "Stmt.h"
#include "VarDecl.h"
#include "TypeBase.h"
#include "Expr.h"
#include "OperatorExpr.h" //TODO remove
#include "Basic/TokenKinds.h"

namespace fly {

    /**
     * Var Declaration
     * Ex.
     *  int a = 1
     */
    class VarDeclStmt : public VarDecl, public Stmt {

        friend class Parser;
        friend class GlobalVarParser;

        const StmtKind Kind = StmtKind::STMT_VAR_DECL;

    public:
        VarDeclStmt(const SourceLocation &Loc, TypeBase *Type, const llvm::StringRef Name);

        StmtKind getKind() const;
    };

    /**
     * Reference to Var Declaration
     * Ex.
     *  ... = a + 1
     */
    class VarRef {

        friend class Parser;

        const llvm::StringRef Name;
        VarDeclStmt *Var = NULL;

    public:
        VarRef(const SourceLocation &Loc, const llvm::StringRef &Name);
        VarRef(const SourceLocation &Loc, VarDeclStmt *D);

        const llvm::StringRef &getName() const;

        VarDecl *getVarDecl() const;

    };

    /**
     * Declaration of a reference to a Var
     * Ex.
     *  a = 1
     */
    class VarAssignStmt : public VarRef, public Stmt {

        friend class Parser;

        GroupExpr *Expr;

    public:
        VarAssignStmt(const SourceLocation &Loc, const llvm::StringRef &Name);
        VarAssignStmt(const SourceLocation &Loc, VarDeclStmt *D);

        StmtKind getKind() const override;

        GroupExpr *getExpr() const;
    };
}

#endif //FLY_VARDECLSTMT_H
