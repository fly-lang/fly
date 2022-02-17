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

    /**
     * Base Var used in:
     *  - LocalVar
     *  - GlobalVar
     */
    class ASTVar {

        friend class ASTNode;
        friend class Parser;
        friend class GlobalVarParser;
        friend class FunctionParser;
        friend class ASTLocalVar;

    protected:
        ASTType *Type;
        const std::string NameSpaceStr;
        const std::string Name;
        bool Constant = false;
        bool Global = false;

    public:
        ASTVar(ASTType *Type, const std::string &Name, const std::string &NameSpaceStr = "", bool Global = false);

        virtual ~ASTVar();

        virtual CodeGenVar *getCodeGen() const = 0;

        bool isGlobal() const;

        virtual bool isConstant() const;

        virtual ASTType *getType() const;

        virtual const std::string &getName() const;

        const std::string &getPrefix() const;

        virtual void setExpr(ASTExpr *Exp) = 0;

        virtual ASTExpr *getExpr() const = 0;

        virtual std::string str() const;

    };

    /**
     * Reference to ASTVar declaration
     * Ex.
     *  ... = a + ...
     *  b = ...
     */
    class ASTVarRef {

        friend class Parser;

        const SourceLocation Loc;
        std::string NameSpace;
        std::string Name;

        ASTVar *Decl = nullptr;

    public:
        ASTVarRef(const SourceLocation &Loc, const std::string &Name, const std::string &NameSpace = "");

        ASTVarRef(ASTVar *Var);

        const SourceLocation &getLocation() const;

        const std::string &getNameSpace() const;

        const std::string &getName() const;

        ASTVar *getDecl() const;

        void setDecl(ASTVar *Var);

        std::string str() const;
    };
}

#endif //FLY_ASTVAR_H
