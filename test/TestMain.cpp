//===--- utils/unittest/UnitTestMain/TestMain.cpp - unittest driver -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#define BACKWARD_HAS_BFD 1

#include "gtest/gtest.h"
//#include "backward.hpp"

int main(int Argc, char **Argv) {

    // Initialize both gmock and gtest.
    ::testing::InitGoogleTest(&Argc, Argv);

    return RUN_ALL_TESTS();
}
