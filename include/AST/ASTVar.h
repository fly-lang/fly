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

#include "Basic/SourceLocation.h"

#include <string>

namespace fly {

    class SourceLocation;
    class CodeGenVar;
    class ASTType;
    class ASTExpr;

    enum class ASTVarKind {
        VAR_LOCAL,
        VAR_GLOBAL,
        VAR_FIELD
    };

    /**
     * Base Var used in:
     *  - LocalVar
     *  - GlobalVar
     */
    class ASTVar {

        friend class SemaBuilder;
        friend class SemaResolver;

        ASTVarKind VarKind;

    protected:

        ASTType *Type = nullptr;

        const std::string Name;

        ASTVar(ASTVarKind VarKind, ASTType *Type, const std::string Name);

        virtual ~ASTVar();

    public:

        ASTVarKind getVarKind();

        ASTType *getType() const;

        const std::string getName() const;

        virtual ASTExpr *getExpr() const = 0;

        virtual CodeGenVar *getCodeGen() const = 0;

        virtual std::string str() const;

    };
}

#endif //FLY_ASTVAR_H
