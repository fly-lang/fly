//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/VarDecl.h - Variable declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_VARDECL_H
#define FLY_VARDECL_H

#include "Decl.h"
#include "Refer.h"
#include "TypeDecl.h"
#include "Expr.h"
#include "OperatorExpr.h" //TODO remove
#include "Basic/TokenKinds.h"

namespace fly {

    class VarDecl : public DeclBase, public Refer {

        friend class Parser;
        friend class GlobalVarParser;

        const DeclKind Kind = DeclKind::D_VAR;
        const TypeDecl *Type;
        const StringRef Name;
        bool Constant = false;
        Expr *Expression = NULL;

    public:
        VarDecl(const SourceLocation &Loc, const TypeDecl *Type, const StringRef Name) :
                DeclBase(Loc), Type(Type), Name(Name) {}

        DeclKind getKind() {
            return Kind;
        }

        bool isConstant() const {
            return Constant;
        }

        const TypeDecl* getType() const {
            return Type;
        }

        const llvm::StringRef &getName() const {
            return Name;
        }

        Expr *getExpr() const {
            return Expression;
        }

        ~VarDecl() {
            delete Type;
        }
    };
}

#endif //FLY_IMPORTDECL_H
