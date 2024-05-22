//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTGlobalVar.h - AST Global Var header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_GLOBALVAR_H
#define FLY_AST_GLOBALVAR_H

#include "ASTTopDef.h"
#include "ASTVar.h"
#include "CodeGen/CodeGenGlobalVar.h"

namespace fly {

    class ASTGlobalVar : public ASTVar, public virtual ASTTopDef {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        ASTTopDefKind TopDefKind = ASTTopDefKind::DEF_GLOBALVAR;

        ASTModule *Module = nullptr;

        ASTExpr *Expr = nullptr;

        // Code Generator
        CodeGenGlobalVar *CodeGen = nullptr;

        ASTGlobalVar(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, ASTScopes *Scopes);

    public:

        ~ASTGlobalVar() = default;

        ASTTopDefKind getTopDefKind() const override;

        ASTModule *getModule() const override;

        ASTNameSpace *getNameSpace() const override;

        llvm::StringRef getName() const override;

        ASTExpr *getExpr() const;

        CodeGenGlobalVar *getCodeGen() const override;

        void setCodeGen(CodeGenGlobalVar *CG);

        std::string print() const override;

        std::string str() const override;

    };
}

#endif //FLY_AST_GLOBALVAR_H
