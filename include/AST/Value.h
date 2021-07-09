//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/Value.h - Var declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_VALUE_H
#define FLY_VALUE_H

#include <llvm/ADT/StringRef.h>
#include "TypeBase.h"

namespace fly {

    class Value {

        const SourceLocation &Loc;

        llvm::StringRef Str;

        TypeBase *Ty;

    public:
        Value(llvm::StringRef Str, TypeBase *Ty);

        const StringRef &str() const;

        TypeBase *getType() const;
    };
}

#endif //FLY_VALUE_H
