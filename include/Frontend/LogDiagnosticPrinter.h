//===--------------------------------------------------------------------------------------------------------------===//
// include/Frontend/LogDiagnosticPrinter.h - Log Diagnostic Client
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_FRONTEND_LOGDIAGNOSTICPRINTER_H
#define FLY_FRONTEND_LOGDIAGNOSTICPRINTER_H

#include "Basic/Diagnostic.h"
#include "Basic/SourceLocation.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include <string>
#include <vector>

namespace fly {

    class DiagnosticOptions;

    class LogDiagnosticPrinter : public DiagnosticConsumer {
public:
  enum class LogFormat { Txt, Json };

  struct InvocationInfo {
    std::vector<std::string> InputFiles;
    std::string Target;
    std::string TargetCpu;
    std::string McModel;
    std::string MthreadModel;
    std::string WorkingDir;
    bool OutputLib   = false;
    bool Verbose     = false;
    bool NoWarnings  = false;
    bool EmitLL      = false;
    bool EmitBC      = false;
    bool EmitAS      = false;
    bool NoOutput    = false;
    bool HeaderGen   = false;
    bool PrintStats  = false;
    bool FtimeReport = false;
  };

private:
  struct DiagEntry {
    /// The primary message line of the diagnostic.
    std::string Message;

    /// The source file name, if available.
    std::string Filename;

    /// The source file line number, if available.
    unsigned Line;

    /// The source file column number, if available.
    unsigned Column;

    /// The ID of the diagnostic.
    unsigned DiagnosticID;

    /// The Option Flag for the diagnostic
    std::string WarningOption;

    /// The level of the diagnostic.
    DiagnosticsEngine::Level DiagnosticLevel;
  };

  void EmitDiagEntry(llvm::raw_ostream &OS,
                     const LogDiagnosticPrinter::DiagEntry &DE);

  // Conditional ownership (when StreamOwner is non-null, it's keeping OS
  // alive). We might want to replace this with a wrapper for conditional
  // ownership eventually - it seems to pop up often enough.
  raw_ostream &OS;
  std::unique_ptr<raw_ostream> StreamOwner;
  IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts;

  SourceLocation LastWarningLoc;
  FullSourceLoc LastLoc;

  SmallVector<DiagEntry, 8> Entries;

  std::string MainFilename;
  std::string DwarfDebugFlags;
  LogFormat Format = LogFormat::Txt;
  InvocationInfo Invocation;

public:
  LogDiagnosticPrinter(raw_ostream &OS, DiagnosticOptions *Diags,
                       std::unique_ptr<raw_ostream> StreamOwner);

  void setMainFilename(StringRef Value) {
    MainFilename = std::string(Value);
  }

  void setDwarfDebugFlags(StringRef Value) {
    DwarfDebugFlags = std::string(Value);
  }

  void setLogFormat(LogFormat F) {
    Format = F;
  }

  void setInvocation(const InvocationInfo &Info) {
    Invocation = Info;
  }

  void BeginSourceFile() override {

  }

  void EndSourceFile() override;

  void HandleDiagnostic(DiagnosticsEngine::Level DiagLevel,
                        const Diagnostic &Info) override;
};

} // end namespace fly

#endif
