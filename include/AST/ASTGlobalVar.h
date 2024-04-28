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

        ASTTopDefKind TopDefKind = ASTTopDefKind::DEF_GLOBALVAR;

        ASTNode *Node;

        ASTVarStmt *Init = nullptr;

        // Code Generator
        CodeGenGlobalVar *CodeGen = nullptr;

        ASTGlobalVar(const SourceLocation &Loc, ASTNode *Node, ASTType *Type, llvm::StringRef Name,
                     ASTScopes *Scopes);

    public:

        ~ASTGlobalVar() = default;

        ASTTopDefKind getTopDefKind() const override;

        ASTNode *getNode() const override;

        ASTNameSpace *getNameSpace() const override;

        llvm::StringRef getName() const override;

        ASTVarStmt *getInit() const;

        void setInit(ASTVarStmt *varDefine);

        CodeGenGlobalVar *getCodeGen() const override;

        void setCodeGen(CodeGenGlobalVar *CG);

        std::string print() const override;

        std::string str() const override;

    };
}

#endif //FLY_AST_GLOBALVAR_H
