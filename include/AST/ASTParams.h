//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTFunc.h - Function declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_ASTPARAMS_H
#define FLY_ASTPARAMS_H

#include "ASTLocalVar.h"

namespace fly {

    /**
     * Function Parameter
     */
    class ASTParam : public ASTLocalVar {

        friend class SemaBuilder;

    public:

        ASTParam(const SourceLocation &Loc, ASTType *Type, const std::string &Name, bool Costant);

        std::string str() const override;
    };

    /**
     * All Parameters of a Function for Definition
     * Ex.
     *   func(int param1, float param2, bool param3, ...)
     */
    class ASTParams {

        friend class SemaBuilder;

        std::vector<ASTParam *> List;
        ASTParam* Ellipsis = nullptr;

    public:
        const std::vector<ASTParam *> &getList() const;

        const ASTParam* getEllipsis() const;
    };
}

#endif //FLY_ASTPARAMS_H
