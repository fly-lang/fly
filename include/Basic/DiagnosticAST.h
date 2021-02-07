//===--- DiagnosticAST.h - Diagnostics for the AST library ------*- C++ -*-===//
//
// Part of the Fly Project, under the Apache License v2.0
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_FLY_BASIC_DIAGNOSTICAST_H
#define LLVM_FLY_BASIC_DIAGNOSTICAST_H

#include "Basic/Diagnostic.h"

namespace fly {
namespace diag {
enum {
#define DIAG(ENUM, FLAGS, DEFAULT_MAPPING, DESC, GROUP, SFINAE, NOWERROR,      \
             SHOWINSYSHEADER, CATEGORY)                                        \
  ENUM,
#define ASTSTART
#include "Basic/DiagnosticASTKinds.inc"
#undef DIAG
  NUM_BUILTIN_AST_DIAGNOSTICS
};
} // end namespace diag
} // end namespace clang

#endif // LLVM_FLY_BASIC_DIAGNOSTICAST_H
