//===--- DiagnosticParse.h - Diagnostics for libparse -----------*- C++ -*-===//
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_FLY_BASIC_DIAGNOSTICPARSE_H
#define LLVM_FLY_BASIC_DIAGNOSTICPARSE_H

#include "Basic/Diagnostic.h"

namespace fly {
namespace diag {
enum {
#define DIAG(ENUM, FLAGS, DEFAULT_MAPPING, DESC, GROUP, SFINAE, NOWERROR,      \
             SHOWINSYSHEADER, CATEGORY)                                        \
  ENUM,
#define PARSESTART
#include "Basic/DiagnosticParseKinds.inc"
#undef DIAG
  NUM_BUILTIN_PARSE_DIAGNOSTICS
};
} // end namespace diag
} // end namespace clang

#endif // LLVM_FLY_BASIC_DIAGNOSTICPARSE_H
