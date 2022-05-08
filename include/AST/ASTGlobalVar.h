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

#include <ostream>
#include "ASTTopDecl.h"
#include "ASTVar.h"
#include "CodeGen/CodeGenGlobalVar.h"

namespace fly {

    class CodeGenGlobalVar;

    class ASTGlobalVar : public ASTVar, public ASTTopDecl {

        friend class SemaBuilder;

        // Code Generator
        CodeGenGlobalVar *CodeGen;

        // Value Expression
        ASTValueExpr *Expr = nullptr;

    public:

        ASTGlobalVar(SourceLocation &Loc, ASTNode *Node, ASTType *Type, const std::string &Name,
                     VisibilityKind Visibility = V_DEFAULT, bool Constant = false);

        ~ASTGlobalVar() = default;

        const std::string &getName() const override;

        CodeGenGlobalVar *getCodeGen() const;

        void setCodeGen(CodeGenGlobalVar *codeGen);

        std::string str() const override;

    };
}

#endif //FLY_PACKAGEVAR_H
