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
        const llvm::StringRef NameSpaceStr;
        const llvm::StringRef Name;
        bool Constant = false;
        bool Global = false;

    public:
        ASTVar(ASTType *Type, const StringRef &Name, const StringRef &NameSpaceStr = "", bool Global = false);

        virtual ~ASTVar();

        virtual CodeGenVar *getCodeGen() const = 0;

        bool isGlobal() const;

        virtual bool isConstant() const;

        virtual ASTType *getType() const;

        virtual const llvm::StringRef &getName() const;

        const llvm::StringRef &getPrefix() const;

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
        llvm::StringRef NameSpace;
        llvm::StringRef Name;

        ASTVar *Decl = nullptr;

    public:
        ASTVarRef(const SourceLocation &Loc, const llvm::StringRef &Name, const llvm::StringRef &NameSpace = "");

        ASTVarRef(ASTVar *Var);

        const SourceLocation &getLocation() const;

        const llvm::StringRef &getNameSpace() const;

        const llvm::StringRef &getName() const;

        ASTVar *getDecl() const;

        void setDecl(ASTVar *decl);

        std::string str() const;
    };
}

#endif //FLY_ASTVAR_H
