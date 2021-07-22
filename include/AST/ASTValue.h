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

#include <llvm/ADT/StringRef.h>
#include "TypeBase.h"

namespace fly {

    class ASTValue {

        const SourceLocation &Loc;

        llvm::StringRef Str;

        TypeBase *Ty;

    public:
        ASTValue(const SourceLocation &Loc, llvm::StringRef Str, TypeBase *Ty);

        const StringRef &str() const;

        TypeBase *getType() const;

        bool empty() const;

        bool isFalse() const;

        bool isTrue() const;
    };
}

#endif //FLY_ASTVALUE_H
