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

#include "Basic/Debuggable.h"
#include "Basic/SourceLocation.h"

#include <string>

namespace fly {

    class SourceLocation;
    class CodeGenVarBase;
    class ASTType;
    class ASTScopes;
    class ASTVarDefine;

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
    class ASTVar : public virtual Debuggable {

        ASTVarKind VarKind;

        const SourceLocation Loc;

        ASTType *Type;

        llvm::StringRef Name;

        ASTScopes *Scopes;

        ASTVarDefine *Initialization = nullptr;

        llvm::StringRef Comment;

    protected:

        ASTVar(ASTVarKind VarKind, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, ASTScopes *Scopes);

    public:

        ASTVarKind getVarKind();

        ASTType *getType() const;

        llvm::StringRef getName() const;

        const SourceLocation &getLocation() const;

        llvm::StringRef getComment() const;

        bool isConstant() const;

        bool isInitialized();

        ASTVarDefine *getInitialization();

        void setInitialization(ASTVarDefine *VarDefine);

        ASTScopes *getScopes() const;

        virtual CodeGenVarBase *getCodeGen() const = 0;

        virtual std::string print() const = 0;

        std::string str() const;
    };
}

#endif //FLY_ASTVAR_H
