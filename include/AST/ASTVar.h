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
        VAR_FIELD
    };

    /**
     * Base Var used in:
     *  - LocalVar
     *  - GlobalVar
     */
    class ASTVar {

        friend class SemaBuilder;
        friend class SemaResolver;

        ASTVarKind VarKind;

    protected:

        ASTType *Type = nullptr;

        const std::string Name;

        ASTVar(ASTVarKind VarKind, ASTType *Type, const std::string Name);

        virtual ~ASTVar();

    public:

        ASTVarKind getVarKind();

        ASTType *getType() const;

        const std::string getName() const;

        virtual ASTExpr *getExpr() const = 0;

        virtual CodeGenVar *getCodeGen() const = 0;

        virtual std::string str() const;

    };

    /**
     * Reference to ASTVar definition
     * Ex.
     *  ... = a + ...
     *  b = ...
     */
    class ASTVarRef {

        friend class SemaBuilder;
        friend class SemaResolver;

        const SourceLocation Loc;
        const std::string NameSpace;
        const std::string Name;

        ASTVar *Def = nullptr;

    public:
        ASTVarRef(const SourceLocation &Loc, const std::string Name, const std::string NameSpace = "");

        const SourceLocation &getLocation() const;

        const std::string getNameSpace() const;

        const std::string getName() const;

        ASTVar *getDef() const;

        std::string str() const;
    };
}

#endif //FLY_ASTVAR_H
