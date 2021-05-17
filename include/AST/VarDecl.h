//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/VarDecl.h - Var declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_VARDECL_H
#define FLY_VARDECL_H

#include "TypeBase.h"
#include "Expr.h"

namespace fly {

    class VarDecl {

        friend class ASTNode;
        friend class Parser;
        friend class GlobalVarParser;
        friend class FunctionParser;
        friend class VarDeclStmt;

        TypeBase *Type;
        const llvm::StringRef Name;
        bool Constant = false;
        GroupExpr *Expression = NULL;

    public:
        VarDecl(TypeBase *Type, const StringRef &Name);
        virtual ~VarDecl();

        virtual bool isConstant() const;

        virtual TypeBase *getType() const;

        virtual const llvm::StringRef &getName() const;

        GroupExpr *getExpr() const;
    };
}

#endif //FLY_VARDECL_H
