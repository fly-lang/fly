//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTLocalVar.h - AST Local Variable statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_ASTLOCALVAR_H
#define FLY_ASTLOCALVAR_H

#include "ASTStmt.h"
#include "ASTVar.h"

namespace fly {

    class CodeGenVar;

    /**
     * Local Var Declaration
     * Ex.
     *  int a = 1
     */
    class ASTLocalVar : public ASTVar, public ASTExprStmt {

        friend class Parser;
        friend class GlobalVarParser;

        const StmtKind Kind = StmtKind::STMT_VAR_DECL;

        unsigned long Order;

        CodeGenVar *CodeGen;

    public:
        ASTLocalVar(const SourceLocation &Loc, ASTBlock *Block, ASTType *Type, const llvm::StringRef &Name);

        StmtKind getKind() const;

        unsigned long getOrder() const;

        void setOrder(unsigned long order);

        ASTExpr *getExpr() const;

        void setExpr(ASTExpr *E);

        CodeGenVar *getCodeGen() const;

        void setCodeGen(CodeGenVar *CG);
    };

    /**
     * Assign somethings to a Local Var
     * Ex.
     *  a = 1
     */
    class ASTLocalVarStmt : public ASTVarRef, public ASTExprStmt {

    public:
        ASTLocalVarStmt(const SourceLocation &Loc, ASTBlock *Block, const llvm::StringRef &Name,
                        const llvm::StringRef &NameSpace = "");

        StmtKind getKind() const override;
    };
}

#endif //FLY_ASTLOCALVAR_H
