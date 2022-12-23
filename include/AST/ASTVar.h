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
    class ASTExpr;

    enum class ASTVarKind {
        VAR_LOCAL,
        VAR_GLOBAL,
        VAR_CLASS
    };

    /**
     * Base Var used in:
     *  - LocalVar
     *  - GlobalVar
     */
    class ASTVar : public virtual Debuggable {

    public:

        virtual ASTVarKind getVarKind() = 0;

        virtual ASTType *getType() const = 0;

        virtual llvm::StringRef getName() const = 0;

        virtual ASTExpr *getExpr() const = 0;

        virtual CodeGenVarBase *getCodeGen() const = 0;
    };
}

#endif //FLY_ASTVAR_H
