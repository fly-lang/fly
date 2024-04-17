//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTVar.h - Var declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ASTVAR_H
#define FLY_ASTVAR_H

#include "ASTBase.h"

#include <string>

namespace fly {

    class SourceLocation;
    class CodeGenVarBase;
    class ASTType;
    class ASTScopes;
    class ASTVarStmt;

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

        ASTVarKind VarKind;

        ASTType *Type;

        llvm::StringRef Name;

        ASTScopes *Scopes;

        ASTVarStmt *Initialization = nullptr;

    protected:

        ASTVar(ASTVarKind VarKind, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, ASTScopes *Scopes);

    public:

        ASTVarKind getVarKind();

        ASTType *getType() const;

        llvm::StringRef getName() const;

        bool isConstant() const;

        bool isInitialized();

        ASTVarStmt *getInitialization();

        void setInitialization(ASTVarStmt *VarDefine);

        ASTScopes *getScopes() const;

        virtual CodeGenVarBase *getCodeGen() const = 0;

        virtual std::string print() const = 0;

        std::string str() const;
    };
}

#endif //FLY_ASTVAR_H
