//===- Basic/PrettyStackTrace.h - Pretty Crash Handling --*- C++ -*-===//
//
// Part of the Fly Project, under the Apache License v2.0
// See https://flylang.org/LICENSE.txt for license information.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Defines the PrettyStackTraceEntry class, which is used to make
/// crashes give more contextual information about what the program was doing
/// when it crashed.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_FLY_BASIC_PRETTYSTACKTRACE_H
#define LLVM_FLY_BASIC_PRETTYSTACKTRACE_H

#include "Basic/SourceLocation.h"
#include "llvm/Support/PrettyStackTrace.h"

namespace fly {

  /// If a crash happens while one of these objects are live, the message
  /// is printed out along with the specified source location.
  class PrettyStackTraceLoc : public llvm::PrettyStackTraceEntry {
    SourceManager &SM;
    SourceLocation Loc;
    const char *Message;
  public:
    PrettyStackTraceLoc(SourceManager &sm, SourceLocation L, const char *Msg)
      : SM(sm), Loc(L), Message(Msg) {}
    void print(raw_ostream &OS) const override;
  };
}

#endif
