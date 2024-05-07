//===--------------------------------------------------------------------------------------------------------------===//
// include/Parser/FunctionParser.h - Function Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_FUNCTIONPARSER_H
#define FLY_FUNCTIONPARSER_H

#include "llvm/ADT/StringRef.h"

namespace fly {

    class Parser;
    class ASTFunction;
    class ASTFunctionBase;
    class ASTCall;
    class ASTType;
    class ASTBlockStmt;
    class SourceLocation;

    class FunctionParser {

        friend class Parser;

        Parser *P;

        ASTFunctionBase *Function;

        unsigned short BraceCount = 0;

        FunctionParser(Parser *P, ASTFunctionBase *Function);

        bool ParseParams();

        bool ParseParam();

        bool ParseBody();

    public:

        static bool Parse(Parser *P, ASTFunctionBase *Function);
    };
}


#endif //FLY_FUNCTIONPARSER_H
