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
    class ASTFunctionCall;
    class ASTType;
    class ASTBlock;
    class SourceLocation;

    class FunctionParser {

        friend class Parser;

        Parser *P;

        ASTFunction *Function = nullptr;

        FunctionParser(Parser *P, VisibilityKind &Visibility, ASTType *Type, bool isHeader);

        bool ParseParams();

        bool ParseParam();

        bool ParseBody();

    public:

        static ASTFunction *Parse(Parser *P, VisibilityKind &Visibility, ASTType *Type, bool isHeader);
    };
}


#endif //FLY_FUNCTIONPARSER_H
