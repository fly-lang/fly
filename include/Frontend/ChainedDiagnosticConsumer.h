//===--------------------------------------------------------------------------------------------------------------===//
// include/Frontend/ChainedDiagnosticConsumer.h - Chain Diagnostic Clients
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_FRONTEND_CHAINEDDIAGNOSTICCONSUMER_H
#define FLY_FRONTEND_CHAINEDDIAGNOSTICCONSUMER_H

#include "Basic/Diagnostic.h"
#include <memory>

namespace fly {

/// ChainedDiagnosticConsumer - Chain two diagnostic clients so that diagnostics
/// go to the first client and then the second. The first diagnostic client
/// should be the "primary" client, and will be used for computing whether the
/// diagnostics should be included in counts.
class ChainedDiagnosticConsumer : public DiagnosticConsumer {
  virtual void anchor();
  std::unique_ptr<DiagnosticConsumer> OwningPrimary;
  DiagnosticConsumer *Primary;
  std::unique_ptr<DiagnosticConsumer> Secondary;

public:
  ChainedDiagnosticConsumer(std::unique_ptr<DiagnosticConsumer> Primary,
                            std::unique_ptr<DiagnosticConsumer> Secondary)
      : OwningPrimary(std::move(Primary)), Primary(OwningPrimary.get()),
        Secondary(std::move(Secondary)) {}

  /// Construct without taking ownership of \c Primary.
  ChainedDiagnosticConsumer(DiagnosticConsumer *Primary,
                            std::unique_ptr<DiagnosticConsumer> Secondary)
      : Primary(Primary), Secondary(std::move(Secondary)) {}

  void BeginSourceFile() override {
    Primary->BeginSourceFile();
    Secondary->BeginSourceFile();
  }

  void EndSourceFile() override {
    Secondary->EndSourceFile();
    Primary->EndSourceFile();
  }

  void finish() override {
    Secondary->finish();
    Primary->finish();
  }

  bool IncludeInDiagnosticCounts() const override {
    return Primary->IncludeInDiagnosticCounts();
  }

  void HandleDiagnostic(DiagnosticsEngine::Level DiagLevel,
                        const Diagnostic &Info) override {
    // Default implementation (Warnings/errors count).
    DiagnosticConsumer::HandleDiagnostic(DiagLevel, Info);

    Primary->HandleDiagnostic(DiagLevel, Info);
    Secondary->HandleDiagnostic(DiagLevel, Info);
  }
};

} // end namspace fly

#endif
