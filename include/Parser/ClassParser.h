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

    class ClassParser {

        friend class Parser;

        Parser *P;

        ASTClass *Class = nullptr;

        ClassParser(Parser *P, VisibilityKind &Visibility, bool &Constant);

    public:

        static ASTClass *Parse(Parser *P, VisibilityKind &Visibility, bool &Constant);
    };
}

#endif //FLY_CLASSPARSER_H
