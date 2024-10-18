//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTVar.h - AST Var header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_VAR_H
#define FLY_AST_VAR_H

#include "ASTBase.h"

namespace fly {

    class SourceLocation;
    class CodeGenVarBase;
    class ASTType;
    class ASTAssignmentStmt;
    class ASTScope;

    enum class ASTVarKind {
        VAR_PARAM,
        VAR_LOCAL,
        VAR_GLOBAL,
        VAR_CLASS,
        VAR_ENUM
    };

    /**
     * Base Var used in:
     *  - LocalVar
     *  - GlobalVar
     */
    class ASTVar : public ASTBase {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        ASTVarKind VarKind;

        ASTType *Type;

        llvm::StringRef Name;

        SmallVector<ASTScope *, 8> Scopes;

        ASTAssignmentStmt *Initialization = nullptr;

    protected:

        ASTVar(ASTVarKind VarKind, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, SmallVector<ASTScope *, 8> &Scopes);

    public:

        ASTVarKind getVarKind();

        ASTType *getType() const;

        llvm::StringRef getName() const;

        bool isConstant() const;

        bool isInitialized();

        ASTAssignmentStmt *getInitialization();

        void setInitialization(ASTAssignmentStmt *VarDefine);

        const SmallVector<ASTScope *, 8> &getScopes() const;

        virtual CodeGenVarBase *getCodeGen() const = 0;

        std::string str() const override;
    };
}

#endif //FLY_AST_VAR_H
