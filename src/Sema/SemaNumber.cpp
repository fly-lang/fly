//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaNumber.cpp - Sema Number
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaNumber.h"
#include "AST/ASTValue.h"
#include "Basic/Debug.h"

using namespace fly;

ASTValue *SemaNumber::fromString(const SourceLocation &Loc, std::string Str) {
    FLY_DEBUG_MESSAGE("SemaNumber", "fromString", "Str=" + Str);

    // TODO Check Hex Value
    bool IsNegative = Str[0] == '-';
    uint64_t Integer = 0;
    uint64_t Fraction = 0;
    bool IsFloatingPoint = false;
    for (unsigned I = 0; I < Str.size(); I++) {
        if (Str[I] == '.') { // check if contain decimals
            Integer = 0;
            IsFloatingPoint = true;
            continue;
        }

        unsigned char N = Str[I] - '0';
        if (N > 9) {
            // Error: not a number
        }
        // subtract '0' from ASCII value of a digit, to obtain integer value of the digit.
        if (IsFloatingPoint) {
            Fraction += Fraction; // need only for zero verify
        }
        else {
            Integer = Integer * 10 + N;
        }
    }

    if (IsFloatingPoint) {
        return new ASTFloatingValue(Loc, Str);
    }
    return new ASTIntegerValue(Loc, Integer, IsNegative);
}

