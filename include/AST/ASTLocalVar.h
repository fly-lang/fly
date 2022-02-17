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
#include "CodeGen/CodeGenLocalVar.h"

namespace fly {

    /**
     * Local Var Declaration
     * Ex.
     *  int a = 1
     */
    class ASTLocalVar : public ASTVar, public ASTExprStmt {

        friend class Parser;
        friend class GlobalVarParser;

        // Statement Kind
        const StmtKind Kind = StmtKind::STMT_VAR_DECL;

        // LocalVar Code Generator
        CodeGenLocalVar *CodeGen = nullptr;

    public:
        ASTLocalVar(const SourceLocation &Loc, ASTBlock *Block, ASTType *Type, const std::string Name);

        StmtKind getKind() const;

        ASTExpr *getExpr() const;

        void setExpr(ASTExpr *E);

        CodeGenLocalVar *getCodeGen() const;

        void setCodeGen(CodeGenLocalVar *CG);

        std::string str() const;
    };

    /**
     * Assign somethings to a Local Var
     * Ex.
     *  a = 1
     */
    class ASTLocalVarRef : public ASTVarRef, public ASTExprStmt {

    public:
        ASTLocalVarRef(const SourceLocation &Loc, ASTBlock *Block, const std::string &Name,
                       const std::string &NameSpace = "");

        ASTLocalVarRef(const SourceLocation &Loc, ASTBlock *Block, ASTVarRef Var);

        StmtKind getKind() const override;

        std::string str() const override;
    };
}

#endif //FLY_ASTLOCALVAR_H
