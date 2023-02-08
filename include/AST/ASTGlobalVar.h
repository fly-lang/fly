//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTGlobalVar.h - Global Var declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_GLOBALVAR_H
#define FLY_GLOBALVAR_H

#include "ASTTopDef.h"
#include "ASTVar.h"
#include "CodeGen/CodeGenGlobalVar.h"

#include <ostream>

namespace fly {

    class ASTGlobalVar : public ASTTopDef, public ASTVar {

        friend class SemaBuilder;

        ASTVarKind VarKind;

        ASTType *Type = nullptr;

        llvm::StringRef Name;

        // Source Location
        const SourceLocation Location;

        ASTExpr *Expr = nullptr;

        // Code Generator
        CodeGenGlobalVar *CodeGen;

        ASTGlobalVar(const SourceLocation &Loc, ASTNode *Node, ASTType *Type, llvm::StringRef Name,
                     ASTTopScopes *Scopes);

    public:

        ~ASTGlobalVar() = default;

        ASTVarKind getVarKind() override;

        ASTType *getType() const override;

        llvm::StringRef getName() const override;

        const SourceLocation &getLocation() const;

        ASTExpr *getExpr() const override;

        CodeGenGlobalVar *getCodeGen() const override;

        void setCodeGen(CodeGenGlobalVar *CG);

        std::string str() const override;

    };
}

#endif //FLY_GLOBALVAR_H
