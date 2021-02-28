//===--- AllDiagnostics.h - Aggregate Diagnostic headers --------*- C++ -*-===//
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Includes all the separate Diagnostic headers & some related helpers.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_FLY_BASIC_ALLDIAGNOSTICS_H
#define LLVM_FLY_BASIC_ALLDIAGNOSTICS_H

#include "Basic/DiagnosticAST.h"
#include "Basic/DiagnosticAnalysis.h"
#include "Basic/DiagnosticComment.h"
#include "Basic/DiagnosticCrossTU.h"
#include "Basic/DiagnosticDriver.h"
#include "Basic/DiagnosticFrontend.h"
#include "Basic/DiagnosticLex.h"
#include "Basic/DiagnosticParse.h"
#include "Basic/DiagnosticSema.h"
#include "Basic/DiagnosticSerialization.h"
#include "Basic/DiagnosticRefactoring.h"

namespace fly {
template <size_t SizeOfStr, typename FieldType>
class StringSizerHelper {
  static_assert(SizeOfStr <= FieldType(~0U), "Field too small!");
public:
  enum { Size = SizeOfStr };
};
} // end namespace clang

#define STR_SIZE(str, fieldTy) fly::StringSizerHelper<sizeof(str)-1, \
                                                        fieldTy>::Size

#endif
