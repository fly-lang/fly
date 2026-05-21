//===--------------------------------------------------------------------------------------------------------------===//
// src/Compiler/LogDiagnosticPrinter.cpp - Log Diagnostic Printer
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Frontend/LogDiagnosticPrinter.h"
#include "Basic/DiagnosticOptions.h"
#include "Basic/FileManager.h"
#include "Basic/SourceManager.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
using namespace fly;

LogDiagnosticPrinter::LogDiagnosticPrinter(
    raw_ostream &os, DiagnosticOptions *diags,
    std::unique_ptr<raw_ostream> StreamOwner)
    : OS(os), StreamOwner(std::move(StreamOwner)),
      DiagOpts(diags) {}

static StringRef getLevelName(DiagnosticsEngine::Level Level) {
  switch (Level) {
  case DiagnosticsEngine::Ignored: return "ignored";
  case DiagnosticsEngine::Remark:  return "remark";
  case DiagnosticsEngine::Note:    return "note";
  case DiagnosticsEngine::Warning: return "warning";
  case DiagnosticsEngine::Error:   return "error";
  case DiagnosticsEngine::Fatal:   return "fatal error";
  }
  llvm_unreachable("Invalid DiagnosticsEngine level!");
}

static void escapeJson(llvm::raw_ostream &OS, StringRef S) {
  OS << '"';
  for (char C : S) {
    switch (C) {
    case '"':  OS << "\\\""; break;
    case '\\': OS << "\\\\"; break;
    case '\n': OS << "\\n";  break;
    case '\r': OS << "\\r";  break;
    case '\t': OS << "\\t";  break;
    default:   OS << C;      break;
    }
  }
  OS << '"';
}

void LogDiagnosticPrinter::EndSourceFile() {
  if (Entries.empty())
    return;

  SmallString<512> Msg;
  llvm::raw_svector_ostream OS(Msg);

  if (Format == LogFormat::Json) {
    OS << "{\n";

    // inputs
    OS << "  \"inputs\": [";
    for (size_t I = 0; I < Invocation.InputFiles.size(); ++I) {
      if (I > 0) OS << ", ";
      escapeJson(OS, Invocation.InputFiles[I]);
    }
    OS << "],\n";

    // output
    if (!MainFilename.empty()) {
      OS << "  \"output\": ";
      escapeJson(OS, MainFilename);
      OS << ",\n";
    }

    // options
    OS << "  \"options\": {\n";
    auto emitStrOpt = [&](StringRef key, const std::string &val) {
      if (!val.empty()) {
        OS << "    "; escapeJson(OS, key); OS << ": "; escapeJson(OS, val); OS << ",\n";
      }
    };
    auto emitBoolOpt = [&](StringRef key, bool val, bool last = false) {
      OS << "    "; escapeJson(OS, key); OS << ": " << (val ? "true" : "false");
      OS << (last ? "\n" : ",\n");
    };
    emitStrOpt("target",         Invocation.Target);
    emitStrOpt("target-cpu",     Invocation.TargetCpu);
    emitStrOpt("mcmodel",        Invocation.McModel);
    emitStrOpt("mthread-model",  Invocation.MthreadModel);
    emitStrOpt("working-dir",    Invocation.WorkingDir);
    emitBoolOpt("lib",           Invocation.OutputLib);
    emitBoolOpt("verbose",       Invocation.Verbose);
    emitBoolOpt("no-warnings",   Invocation.NoWarnings);
    emitBoolOpt("emit-ll",       Invocation.EmitLL);
    emitBoolOpt("emit-bc",       Invocation.EmitBC);
    emitBoolOpt("emit-as",       Invocation.EmitAS);
    emitBoolOpt("no-output",     Invocation.NoOutput);
    emitBoolOpt("header",        Invocation.HeaderGen);
    emitBoolOpt("print-stats",   Invocation.PrintStats);
    emitBoolOpt("ftime-report",  Invocation.FtimeReport, /*last=*/true);
    OS << "  },\n";

    // diagnostics
    OS << "  \"diagnostics\": [\n";
    for (size_t I = 0; I < Entries.size(); ++I) {
      const DiagEntry &DE = Entries[I];
      OS << "    {\n";
      OS << "      \"level\": ";   escapeJson(OS, getLevelName(DE.DiagnosticLevel)); OS << ",\n";
      if (!DE.Filename.empty()) {
        OS << "      \"file\": ";  escapeJson(OS, DE.Filename); OS << ",\n";
      }
      if (DE.Line != 0)
        OS << "      \"line\": "   << DE.Line   << ",\n";
      if (DE.Column != 0)
        OS << "      \"column\": " << DE.Column << ",\n";
      OS << "      \"message\": "; escapeJson(OS, DE.Message); OS << "\n";
      OS << "    }" << (I + 1 < Entries.size() ? "," : "") << "\n";
    }
    OS << "  ]\n}\n";

  } else {
    // Txt format
    if (!Invocation.InputFiles.empty()) {
      OS << "inputs: ";
      for (size_t I = 0; I < Invocation.InputFiles.size(); ++I) {
        if (I > 0) OS << " ";
        OS << Invocation.InputFiles[I];
      }
      OS << "\n";
    }
    if (!MainFilename.empty())
      OS << "output: " << MainFilename << "\n";
    if (!Invocation.Target.empty())
      OS << "target: " << Invocation.Target << "\n";
    if (!Invocation.TargetCpu.empty())
      OS << "target-cpu: " << Invocation.TargetCpu << "\n";
    if (!Invocation.McModel.empty())
      OS << "mcmodel: " << Invocation.McModel << "\n";
    if (!Invocation.MthreadModel.empty())
      OS << "mthread-model: " << Invocation.MthreadModel << "\n";
    if (!Invocation.WorkingDir.empty())
      OS << "working-dir: " << Invocation.WorkingDir << "\n";

    // Collect active boolean flags
    SmallString<128> Flags;
    llvm::raw_svector_ostream FOS(Flags);
    if (Invocation.OutputLib)   FOS << " --lib";
    if (Invocation.Verbose)     FOS << " --verbose";
    if (Invocation.NoWarnings)  FOS << " --no-warnings";
    if (Invocation.EmitLL)      FOS << " --emit-ll";
    if (Invocation.EmitBC)      FOS << " --emit-bc";
    if (Invocation.EmitAS)      FOS << " --emit-as";
    if (Invocation.NoOutput)    FOS << " --no-output";
    if (Invocation.HeaderGen)   FOS << " --header";
    if (Invocation.PrintStats)  FOS << " --print-stats";
    if (Invocation.FtimeReport) FOS << " --ftime-report";
    if (!Flags.empty())
      OS << "flags:" << Flags << "\n";

    OS << "---\n";

    for (const DiagEntry &DE : Entries) {
      OS << getLevelName(DE.DiagnosticLevel) << ": ";
      if (!DE.Filename.empty()) {
        OS << DE.Filename;
        if (DE.Line != 0) {
          OS << ":" << DE.Line;
          if (DE.Column != 0)
            OS << ":" << DE.Column;
        }
        OS << ": ";
      }
      OS << DE.Message << "\n";
    }
  }

  this->OS << OS.str();
}

void LogDiagnosticPrinter::HandleDiagnostic(DiagnosticsEngine::Level Level,
                                            const Diagnostic &Info) {
  // Default implementation (Warnings/errors count).
  DiagnosticConsumer::HandleDiagnostic(Level, Info);

  // Create the diag entry.
  DiagEntry DE;
  DE.DiagnosticID = Info.getID();
  DE.DiagnosticLevel = Level;

  DE.WarningOption = std::string(DiagnosticIDs::getWarningOptionForDiag(DE.DiagnosticID));

  // Format the message.
  SmallString<100> MessageStr;
  Info.FormatDiagnostic(MessageStr);
  DE.Message = std::string(MessageStr.str());

  // Set the location information.
  DE.Filename = "";
  DE.Line = DE.Column = 0;
  if (Info.getLocation().isValid() && Info.hasSourceManager()) {
    const SourceManager &SM = Info.getSourceManager();
    PresumedLoc PLoc = SM.getPresumedLoc(Info.getLocation());

    if (PLoc.isInvalid()) {
      // At least print the file name if available:
      FileID FID = SM.getFileID(Info.getLocation());
      if (FID.isValid()) {
        const FileEntry *FE = SM.getFileEntryForID(FID);
        if (FE && FE->isValid())
          DE.Filename = std::string(FE->getName());
      }
    } else {
      DE.Filename = PLoc.getFilename();
      DE.Line = PLoc.getLine();
      DE.Column = PLoc.getColumn();
    }
  }

  // Record the diagnostic entry.
  Entries.push_back(DE);
}

