//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTLocalVar.h - AST Local Variable header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_LOCALVAR_H
#define FLY_AST_LOCALVAR_H

#include "ASTVar.h"
#include "CodeGen/CodeGenVar.h"

namespace fly {

    /**
     * Local Var Declaration
     * Ex.
     *  int a = 1
     */
    class ASTLocalVar : public ASTVar {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        CodeGenVarBase *CodeGen = nullptr;

    protected:

        ASTLocalVar(ASTVarKind VarKind, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                    llvm::SmallVector<ASTScope *, 8> &Scopes);

        ASTLocalVar(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name,
                    llvm::SmallVector<ASTScope *, 8> &Scopes);

    public:

        CodeGenVarBase *getCodeGen() const override;

        void setCodeGen(CodeGenVarBase *CG);

        std::string str() const override;
    };
}

#endif //FLY_AST_LOCALVAR_H
