//===--- DiagnosticSerialization.h - Serialization Diagnostics -*- C++ -*-===//
//
// Part of the Fly Project, under the Apache License v2.0
// See https://flylang.org/LICENSE.txt for license information.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_FLY_BASIC_DIAGNOSTICSERIALIZATION_H
#define LLVM_FLY_BASIC_DIAGNOSTICSERIALIZATION_H

#include "Basic/Diagnostic.h"

namespace fly {
namespace diag {
enum {
#define DIAG(ENUM, FLAGS, DEFAULT_MAPPING, DESC, GROUP, SFINAE, NOWERROR,      \
             SHOWINSYSHEADER, CATEGORY)                                        \
  ENUM,
#define SERIALIZATIONSTART
#include "Basic/DiagnosticSerializationKinds.inc"
#undef DIAG
  NUM_BUILTIN_SERIALIZATION_DIAGNOSTICS
};
} // end namespace diag
} // end namespace clang

#endif // LLVM_FLY_BASIC_DIAGNOSTICSERIALIZATION_H
