//===--- utils/unittest/UnitTestMain/TestMain.cpp - unittest driver -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Signals.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

int main(int Argc, char **Argv) {
  llvm::sys::PrintStackTraceOnErrorSignal(Argv[0],true /* Disable crash reporting */);

  // Initialize both gmock and gtest.
  ::testing::InitGoogleMock(&Argc, Argv);

  return RUN_ALL_TESTS();
}
