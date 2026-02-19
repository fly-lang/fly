//===--------------------------------------------------------------------------------------------------------------===//
// test/ParserTest.cpp - Parser tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "ParserTest.h"
#include "llvm/ADT/StringRef.h"

namespace {

    using namespace fly;

    TEST_F(ParserTest, BadColon) {
        llvm::StringRef str = (
                "func() {\n"
                "  a:"
                "  return"
                "}\n");
        Parse("BadColon", str);
    }
}