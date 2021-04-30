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
#include "Lex/Token.h"
#include "AST/ClassDecl.h"

namespace fly {

    class Parser;

    class ClassParser {

        friend class Parser;

        Parser *P;
        ClassDecl *Class = NULL;

    public:
        ClassParser(Parser *P);

        bool Parse();
    };
}

#endif //FLY_CLASSPARSER_H
