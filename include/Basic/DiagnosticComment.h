//===--- DiagnosticComment.h - Diagnostics for the AST library --*- C++ -*-===//
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_FLY_BASIC_DIAGNOSTICCOMMENT_H
#define LLVM_FLY_BASIC_DIAGNOSTICCOMMENT_H

#include "Basic/Diagnostic.h"

namespace fly {
namespace diag {
enum {
#define DIAG(ENUM, FLAGS, DEFAULT_MAPPING, DESC, GROUP, SFINAE, NOWERROR,      \
             SHOWINSYSHEADER, CATEGORY)                                        \
  ENUM,
#define COMMENTSTART
#include "Basic/DiagnosticCommentKinds.inc"
#undef DIAG
  NUM_BUILTIN_COMMENT_DIAGNOSTICS
};
} // end namespace diag
} // end namespace clang

#endif // LLVM_FLY_BASIC_DIAGNOSTICCOMMENT_H
