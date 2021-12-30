//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTValue.h - AST Value
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_ASTVALUE_H
#define FLY_ASTVALUE_H

#include "ASTType.h"

namespace fly {

    class ASTValue {

        const SourceLocation &Loc;

        std::string Str;

        ASTType *Ty;

    public:
        ASTValue(const SourceLocation &Loc, std::string Str, ASTType *Ty);

        ASTType *getType() const;

        bool empty() const;

        bool isFalse() const;

        bool isTrue() const;

        std::string str() const;
    };
}

#endif //FLY_ASTVALUE_H
