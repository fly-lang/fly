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

#include "CodeGen/CGVar.h"
#include "Stmt.h"
#include "VarDecl.h"
#include "TypeBase.h"
#include "Expr.h"
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
        CGVar *CodeGen;

    public:
        VarDeclStmt(const SourceLocation &Loc, BlockStmt *CurrStmt, TypeBase *Type, const llvm::StringRef Name);

        StmtKind getKind() const;

        void setCodeGen(CGVar *CG);
    };
}

#endif //FLY_VARDECLSTMT_H
