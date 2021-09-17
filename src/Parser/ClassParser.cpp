//===--------------------------------------------------------------------------------------------------------------===//
// src/Parser/ClassParser.cpp - Class Parser
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Parser/ClassParser.h"

using namespace fly;

/**
 * ClassParser Constructor
 * @param P
 */
ClassParser::ClassParser(Parser *P) : P(P){

}

/**
 * Parse Class Declaration
 * @return
 */
bool ClassParser::Parse() {
    return true;
}
