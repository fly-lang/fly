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

    public:

        ASTGlobalVar(ASTNode *Node, SourceLocation &Loc, ASTType *Type, const llvm::StringRef &Name);

        TopDeclKind getKind() const override;

        CodeGenGlobalVar *getCodeGen() const;

        void setCodeGen(CodeGenGlobalVar *codeGen);

        ~ASTGlobalVar() = default;
    };
}

#endif //FLY_PACKAGEVAR_H
