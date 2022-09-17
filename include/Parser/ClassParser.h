//===--------------------------------------------------------------------------------------------------------------===//
// include/Parser/ClassParser.h - Class Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CLASSPARSER_H
#define FLY_CLASSPARSER_H

#include "Parser.h"

namespace fly {

    class ASTClass;
    class ASTClassScopes;
    class ASTClassField;
    class ASTClassMethod;

    class ClassParser {

        friend class Parser;

        Parser *P;

        ASTClass *Class = nullptr;

        bool Success = true;

        ClassParser(Parser *P, ASTTopScopes *Scopes);

    public:

        static ASTClass *Parse(Parser *P, ASTTopScopes *Scopes);

        ASTClassScopes *ParseScopes();

        bool isField();

        ASTClassField *ParseField();

        bool isMethod();

        ASTClassMethod *ParseMethod();
    };
}

#endif //FLY_CLASSPARSER_H
