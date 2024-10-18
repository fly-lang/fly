//===--------------------------------------------------------------------------------------------------------------===//
// include/Parser/ClassParser.h - Class Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_PARSERENUM_H
#define FLY_PARSERENUM_H

#include "llvm/ADT/SmallVector.h"
#include <cstdint>

namespace llvm {
    class StringRef;
}

namespace fly {

    class ASTScope;
    class ASTEnum;
    class Parser;
    class SourceLocation;
    class ASTComment;

    class ParserEnum {

        friend class Parser;

        Parser *P;

        ASTEnum *Enum = nullptr;

        bool Success = true;

        unsigned short BraceCount = 0;

        ParserEnum(Parser *P, ASTComment *Comment, llvm::SmallVector<ASTScope *, 8> &Scopes);

    public:

        static ASTEnum *Parse(Parser *P, ASTComment *Comment, llvm::SmallVector<ASTScope *, 8> &Scopes);

        bool ParseField(const SourceLocation &Loc, llvm::StringRef Name, llvm::SmallVector<ASTScope *, 8> Scopes);
    };
}

#endif //FLY_CLASSPARSER_H
