//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/Sema.h - Main Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMANUMBER_H
#define FLY_SEMANUMBER_H

#include <string>

namespace fly {

    class ASTValue;
    class SourceLocation;

    class SemaNumber {

    public:

        static ASTValue *fromString(const SourceLocation &Loc, std::string Val);

    };
}

#endif //FLY_SEMANUMBER_H
