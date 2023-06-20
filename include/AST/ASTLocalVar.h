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
#include "CodeGen/CodeGenVar.h"

namespace fly {

    /**
     * Local Var Declaration
     * Ex.
     *  int a = 1
     */
    class ASTLocalVar : public ASTExprStmt, public ASTVar {

        friend class SemaBuilder;
        friend class SemaResolver;

        // LocalVar Code Generator
        CodeGenVarBase *CodeGen = nullptr;

        bool Constant = false;

        ASTVarKind VarKind;

        ASTType *Type = nullptr;

        llvm::StringRef Name;

    protected:

         ASTLocalVar(ASTBlock *Parent, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, bool Constant);

    public:

        ASTVarKind getVarKind() override;

        ASTType *getType() const override;

        llvm::StringRef getName() const override;

        bool isConstant() const;

        ASTExpr *getExpr() const override;

        CodeGenVarBase *getCodeGen() const override;

        void setCodeGen(CodeGenVarBase *CG);

        std::string print() const override;

        std::string str() const;
    };
}

#endif //FLY_ASTLOCALVAR_H
