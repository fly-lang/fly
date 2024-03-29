//===--------------------------------------------------------------------------------------------------------------===//
// src/Basic/DiagnosticOptions.cpp - C Language Family Diagnostic Handling
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
//
//  This file implements the DiagnosticOptions related interfaces.
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Basic/DiagnosticOptions.h"
#include "llvm/Support/raw_ostream.h"
#include <type_traits>

namespace fly {

raw_ostream &operator<<(raw_ostream &Out, DiagnosticLevelMask M) {
  using UT = std::underlying_type<DiagnosticLevelMask>::type;
  return Out << static_cast<UT>(M);
}

} // namespace clang
