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

#include "ASTExprStmt.h"
#include "ASTVar.h"
#include "CodeGen/CodeGenLocalVar.h"

namespace fly {

    /**
     * Local Var Declaration
     * Ex.
     *  int a = 1
     */
    class ASTLocalVar : public ASTVar, public ASTExprStmt {

        friend class SemaBuilder;
        friend class SemaResolver;

        // Statement Kind
        const StmtKind Kind = StmtKind::STMT_VAR_DEFINE;

        ASTExpr *Expr = nullptr;

        // LocalVar Code Generator
        CodeGenLocalVar *CodeGen = nullptr;

    public:
        ASTLocalVar(const SourceLocation &Loc, ASTType *Type, const std::string &Name, bool Constant);

        StmtKind getKind() const;

        ASTExpr *getExpr() const override;

        CodeGenLocalVar *getCodeGen() const;

        void setCodeGen(CodeGenLocalVar *CG);

        std::string str() const;
    };
}

#endif //FLY_ASTLOCALVAR_H
