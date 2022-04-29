//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/NumberParser.cpp - Number Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/NumberParser.h"
#include "AST/ASTValue.h"

using namespace fly;

/**
 * Parse a Number
 * @return
 */
NumberParser::NumberParser(const SourceLocation &Loc, std::string &Str) : Loc(Loc), Str(Str) {
    Parse();
}

ASTSingleValue *NumberParser::getValue(ASTType *Type) {
    if (Type == nullptr) {
        return getValue();
    }
    if (Type->isInteger()) {
        return new ASTIntegerValue(Loc, Type, Integer, NegativeInt);
    } else if (Type->isFloatingPoint()) {
        return new ASTFloatingValue(Loc, Type, Str);
    } else if (Type->isBool()) {
        return new ASTBoolValue(Loc, Integer != 0 || (isFloatingPoint && Fraction != 0));
    }

    assert("Unknown number value");
}

ASTSingleValue *NumberParser::getValue() {
    if (isFloatingPoint) {
        return new ASTFloatingValue(Loc, new ASTFloatType(Loc), Str);
    } else {
        return new ASTIntegerValue(Loc, new ASTIntType(Loc), Integer, NegativeInt);
    }
}

void NumberParser::Parse() {
    // TODO Check Hex Value
    NegativeInt = Str[0] == '-';
    Integer = 0;
    unsigned J = Str.size();
    for (unsigned I = 0; I < Str.size(); I++, J--) {
        if (Str[I] == '.') { // check if contain decimals
            Integer = 0;
            isFloatingPoint = true;
            continue;
        }

        unsigned char N = Str[I] - '0';
        if (N > 9) {
            // Error: not a number
        }
        // subtract '0' from ASCII value of a digit, to obtain integer value of the digit.
        if (isFloatingPoint) {
            Fraction += Fraction; // need only for zero verify
        }
        else {
            Integer = Integer * 10 + N;
        }
    }
}
