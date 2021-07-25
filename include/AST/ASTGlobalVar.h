//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTGlobalVar.h - Global Var declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_PACKAGEVAR_H
#define FLY_PACKAGEVAR_H

#include "ASTTopDecl.h"
#include "ASTVar.h"

namespace fly {

    class CodeGenGlobalVar;

    class ASTGlobalVar : public ASTVar, public ASTTopDecl {

        friend class ASTNode;
        const TopDeclKind Kind;
        CodeGenGlobalVar *CodeGen;
        ASTValueExpr *Expr = nullptr;

    public:

        ASTGlobalVar(ASTNode *Node, SourceLocation &Loc, ASTType *Type, const llvm::StringRef &Name);

        ~ASTGlobalVar() = default;

        TopDeclKind getKind() const override;

        ASTExpr *getExpr() const override;

        void setExpr(ASTExpr *E) override;

        CodeGenGlobalVar *getCodeGen() const;

        void setCodeGen(CodeGenGlobalVar *codeGen);
    };
}

#endif //FLY_PACKAGEVAR_H
