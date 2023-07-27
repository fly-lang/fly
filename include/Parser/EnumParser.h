//===--------------------------------------------------------------------------------------------------------------===//
// include/Parser/ClassParser.h - Class Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_ENUMPARSER_H
#define FLY_ENUMPARSER_H

#include <cstdint>

namespace llvm {
    class StringRef;
}

namespace fly {

    class ASTEnum;
    class ASTScopes;
    class ASTScopes;
    class Parser;
    class SourceLocation;

    class EnumParser {

        friend class Parser;

        Parser *P;

        ASTEnum *Enum = nullptr;

        bool Success = true;

        unsigned short BraceCount = 0;

        EnumParser(Parser *P, ASTScopes *EnumScopes);

    public:

        static ASTEnum *Parse(Parser *P, ASTScopes *EnumScopes);

        bool ParseField(const SourceLocation &Loc, llvm::StringRef Name);
    };
}

#endif //FLY_CLASSPARSER_H
