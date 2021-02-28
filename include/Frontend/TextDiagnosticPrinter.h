//===----------------------------------------------------------------------===//
// include/Frontend/TextDiagnosticPrinter.h - Text Diagnostic Client
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//
//
// This is a concrete diagnostic client, which prints the diagnostics to
// standard error.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_FRONTEND_TEXTDIAGNOSTICPRINTER_H
#define LLVM_CLANG_FRONTEND_TEXTDIAGNOSTICPRINTER_H

#include "Basic/Diagnostic.h"
#include "Basic/LLVM.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include <memory>

namespace fly {
class DiagnosticOptions;
class TextDiagnostic;

class TextDiagnosticPrinter : public DiagnosticConsumer {
  raw_ostream &OS;
  IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts;

  /// Handle to the currently active text diagnostic emitter.
  std::unique_ptr<TextDiagnostic> TextDiag;

  /// A string to prefix to error messages.
  std::string Prefix;

  unsigned OwnsOutputStream : 1;

public:
  TextDiagnosticPrinter(raw_ostream &os, DiagnosticOptions *diags,
                        bool OwnsOutputStream = false);
  ~TextDiagnosticPrinter() override;

  /// setPrefix - Set the diagnostic printer prefix string, which will be
  /// printed at the start of any diagnostics. If empty, no prefix string is
  /// used.
  void setPrefix(std::string Value) { Prefix = std::move(Value); }

  void BeginSourceFile() override;
  void EndSourceFile() override;
  void HandleDiagnostic(DiagnosticsEngine::Level Level,
                        const Diagnostic &Info) override;
};

} // end namespace clang

#endif
