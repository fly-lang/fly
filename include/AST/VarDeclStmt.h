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

namespace fly {

    class CodeGenVar;

    /**
     * Var Declaration
     * Ex.
     *  int a = 1
     */
    class VarDeclStmt : public VarDecl, public Stmt {

        friend class Parser;
        friend class GlobalVarParser;

        const StmtKind Kind = StmtKind::STMT_VAR_DECL;

        unsigned long Order;

        CodeGenVar *CodeGen;

    public:
        VarDeclStmt(const SourceLocation &Loc, BlockStmt *Block, TypeBase *Type, const llvm::StringRef &Name);

        StmtKind getKind() const;

        unsigned long getOrder() const;

        void setOrder(unsigned long order);

        CodeGenVar *getCodeGen() const;

        void setCodeGen(CodeGenVar *CG);
    };
}

#endif //FLY_VARDECLSTMT_H
