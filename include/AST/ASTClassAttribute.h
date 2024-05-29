//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTClassVar.h - AST Class Attribute header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_CLASSVAR_H
#define FLY_AST_CLASSVAR_H

#include "ASTVar.h"
#include "CodeGen/CodeGenClassVar.h"

namespace fly {

    class ASTClass;
    class ASTScopes;
    class ASTType;
    class ASTValue;
    class ASTVar;

    class ASTClassAttribute : public ASTVar {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        ASTClass *Class = nullptr;

        ASTVarStmt *Init = nullptr;

        CodeGenVarBase *CodeGen = nullptr;

        ASTClassAttribute(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, ASTScopes *Scopes);

    public:

        ASTClass *getClass() const;

        ASTVarStmt *getInit() const;

        void setInit(ASTVarStmt *varDefine);

        CodeGenVarBase * getCodeGen() const override;

        void setCodeGen(CodeGenVarBase *CGV);

        std::string str() const override;
    };
}

#endif //FLY_AST_CLASSVAR_H
