//===--------------------------------------------------------------------------------------------------------------===//
// include/Parser/NumberParser.h - Number Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_NUMBERPARSER_H
#define FLY_NUMBERPARSER_H

#include "Basic/SourceLocation.h"
#include <string>
#include <cstdint>

namespace fly {

    class SourceLocation;
    class ASTBoolValue;
    class ASTIntegerValue;
    class ASTFloatingValue;
    class ASTSingleValue;
    class ASTType;

    class NumberParser {

        const SourceLocation Loc;
        std::string Str;
        bool NegativeInt; // true is negative and false is positive
        uint64_t Integer;
        uint64_t Fraction;
        bool isFloatingPoint;

    public:
        NumberParser(const SourceLocation &Loc, std::string &Str);

        ASTSingleValue *getValue(ASTType *Type);

        ASTSingleValue *getValue();

    private:

        void Parse();
    };
}

#endif //FLY_NUMBERPARSER_H
