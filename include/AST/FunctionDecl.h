//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/FunctionDecl.h - Function declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_FUNCTION_H
#define FLY_FUNCTION_H

#include "VarDecl.h"
#include "Refer.h"
#include "Expr.h"
#include "TypeDecl.h"
#include "Stmt.h"
#include "vector"

namespace fly {

    class ParamsFunc;

    class FunctionDecl : public DeclBase, public Refer, public TopDecl {

        friend class ASTNode;
        friend class Parser;
        friend class FunctionParser;

        const TypeDecl *Type;
        const StringRef &Name;
        bool Constant;
        ParamsFunc *Params = NULL;
        Stmt *Body = NULL;

    public:
        FunctionDecl(const SourceLocation &Loc, const TypeDecl *Type, const StringRef &Name);

        DeclKind getKind() override {
            return DeclKind::D_FUNCTION;
        }

        const TypeDecl *getType() const;

        const StringRef &getName() const;

        bool isConstant() const;

        const ParamsFunc *getParams() const;

        const Stmt *getBody() const;

    };

    class ParamsFunc {

        friend class FunctionParser;

        std::vector<VarDecl*> Vars;
        VarDecl* VarArg;

    public:
        const std::vector<VarDecl *> &getVars() const;

        const VarDecl* getVarArg() const;
    };

    class ReturnDecl: public DeclBase {

        DeclKind Kind = DeclKind::D_RETURN;
        Expr* Exp;

    public:
        ReturnDecl(SourceLocation &Loc, class Expr *E);

        DeclKind getKind() override;

        class Expr *getExpr() const;
    };
}

#endif //FLY_FUNCTION_H
