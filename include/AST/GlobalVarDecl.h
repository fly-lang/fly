//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/GlobalVarDecl.h - Global Var declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_PACKAGEVAR_H
#define FLY_PACKAGEVAR_H

#include "TopDecl.h"
#include "VarDecl.h"
#include "TypeBase.h"
#include "CodeGen/CGGlobalVar.h"

namespace fly {

    class GlobalVarDecl : public VarDecl, public TopDecl {

        friend class ASTNode;
        const TopDeclKind Kind = TopDeclKind::DECL_GLOBALVAR;
        CGGlobalVar *CodeGen;

    public:

        GlobalVarDecl(SourceLocation &Loc, TypeBase *Type, llvm::StringRef Name);

        TopDeclKind getKind() const override;

        CGGlobalVar *getCodeGen() const;

        void setCodeGen(CGGlobalVar *codeGen);

        ~GlobalVarDecl() = default;
    };
}

#endif //FLY_PACKAGEVAR_H
