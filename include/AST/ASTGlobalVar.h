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

#include "ASTVar.h"
#include "CodeGen/CodeGenGlobalVar.h"

namespace fly {

    class ASTModule;
    class ASTNameSpace;
    class ASTComment;
    enum class ASTVisibilityKind;

    class ASTGlobalVar : public ASTVar {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        ASTVisibilityKind Visibility;

        bool Constant;

        ASTModule *Module;

        ASTExpr *Expr = nullptr;

        // Code Generator
        CodeGenGlobalVar *CodeGen = nullptr;

        ASTGlobalVar(ASTModule *Module, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, SmallVector<ASTScope *, 8> &Scopes);

    public:

        ~ASTGlobalVar() = default;

        ASTVisibilityKind getVisibility() const;

        bool isConstant() const;

        ASTModule *getModule() const;

        ASTNameSpace *getNameSpace() const;

        llvm::StringRef getName() const;

        ASTExpr *getExpr() const;

        CodeGenGlobalVar *getCodeGen() const override;

        void setCodeGen(CodeGenGlobalVar *CG);

        std::string str() const override;

    };
}

#endif //FLY_AST_GLOBALVAR_H
