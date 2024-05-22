//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTFunc.h - Function declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_ASTPARAM_H
#define FLY_ASTPARAM_H

#include "ASTLocalVar.h"

namespace fly {

    class ASTFunctionBase;
    class ASTValue;

    /**
     * Function Parameter
     */
    class ASTParam : public ASTLocalVar {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        ASTValue *DefaultValue = nullptr;

        ASTParam(const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, ASTScopes *Scopes);

    public:

        ASTValue *getDefaultValue() const;

        void setDefaultValue(ASTValue *Value);

        std::string print() const override;

        std::string str() const override;
    };
}

#endif //FLY_ASTPARAM_H
