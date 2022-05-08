//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTVar.h - Var declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTVAR_H
#define FLY_ASTVAR_H

#include "ASTType.h"
#include "ASTExpr.h"
#include "ASTStmt.h"

namespace fly {

    class CodeGenVar;

    enum ASTVarKind {
        VAR_LOCAL,
        VAR_GLOBAL,
        VAR_CLASS
    };

    /**
     * Base Var used in:
     *  - LocalVar
     *  - GlobalVar
     */
    class ASTVar {

        friend class SemaBuilder;

        ASTVarKind VarKind;

    protected:
        ASTType *Type;
        const std::string Name;
        bool Constant = false;
        ASTExpr *Expr = nullptr;

        ASTVar(ASTVarKind VarKind, ASTType *Type, const std::string &Name, bool Constant);

    public:
        virtual ~ASTVar();

        ASTVarKind getVarKind();

        virtual bool isConstant() const;

        virtual ASTType *getType() const;

        virtual const std::string &getName() const;

        ASTExpr *getExpr() const;

        virtual CodeGenVar *getCodeGen() const = 0;

        virtual std::string str() const;

    };

    /**
     * Reference to ASTVar declaration
     * Ex.
     *  ... = a + ...
     *  b = ...
     */
    class ASTVarRef {

        friend class SemaBuilder;

        const SourceLocation Loc;
        const std::string NameSpace;
        const std::string Name;

        ASTVar *Decl = nullptr;

    public:
        ASTVarRef(const SourceLocation &Loc, const std::string &Name, const std::string &NameSpace = "");

        const SourceLocation &getLocation() const;

        const std::string &getNameSpace() const;

        const std::string &getName() const;

        ASTVar *getDecl() const;

        void setDecl(ASTVar *Var);

        std::string str() const;
    };
}

#endif //FLY_ASTVAR_H
