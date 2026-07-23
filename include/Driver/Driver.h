//===--------------------------------------------------------------------------------------------------------------===//
// include/Driver/Driver.h - compiler driver
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_DRIVER_H
#define FLY_DRIVER_H

#include "Frontend/FrontendOptions.h"
#include "Frontend/CompilerInstance.h"
#include "Basic/Diagnostic.h"
#include "Basic/FileSystemOptions.h"
#include <string>
#include <vector>

namespace fly {

    class ToolChain;

    class Driver {

        // The Compiler Instance contains components needs for compilation phase
        std::shared_ptr<CompilerInstance> CI;

        // Can go ahead with execute phase
        bool doExecute = true;

        // A command-line error occurred (unknown option, unexpected positional,
        // conflicting options): Execute() is a no-op that returns FAILURE, so the
        // process exits 1 — matching the self-host driver. --help/--version also
        // clear doExecute but are not errors and keep exit 0.
        bool HadOptionError = false;

        /// The name the driver was invoked as.
        std::string Name;

        /// The path the driver executable was in, as invoked from the command line.
        std::string Dir;

        /// The original path to the fly executable.
        std::string Path;

        /// The path to the installed fly directory, if any.
        std::string InstalledDir;

        /// The path to the compiler resource directory.
        std::string ResourceDir;

        // ── Parsed option values ──────────────────────────────────────────────
        std::vector<std::string> LibDirs;
        std::vector<std::string> SrcDirs;   // --src-dir flags: source search paths for import-based dep discovery
        std::vector<std::string> LinkLibs;  // --link-lib flags: external C libs to link (-lNAME)
        std::string LlvmLibDir;             // --llvm-lib-dir: dir holding the LLVM libs (libLLVM-20.so / LLVM-20.lib)
        std::string OutDirOpt;              // --out-dir: directory for all generated build outputs
        std::string OutputFile;
        // Linked-artifact shape (an axis of its own: it says what the LINK step
        // produces, never what the backend emits — see BuildOptions()).
        bool OutputLib    = false;  // --lib / --lib-static:  static archive
        bool OutputShared = false;  // --lib-dyn / --lib-dynamic: shared library
        std::string LogFile;
        std::string LogFormat;
        std::string WorkingDir;
        std::string McModel;
        std::string MthreadModel;
        unsigned    Jobs    = 0;
        int         OptLevel = -1; // -1 = use default; 0–3 from -O flag
        std::string Target;
        std::string TargetCpu;
        std::string StatsFile;
        bool DebugSymbols = false;
        bool TestMode     = false;
        bool SuiteRun     = false;  // --suite: build the suite test exe, then run it
        std::string SuiteName;      // --suite <Name>: which suite gets the implicit main()
        std::string TestFilter;     // --test <Method>: run only this test-method
        int  RunExitCode  = 0;      // exit code of the executed suite binary
        bool Verbose      = false;
        bool NoWarnings   = false;
        bool EmitLL       = false;
        bool EmitBC       = false;
        bool EmitAS       = false;
        bool NoOutput     = false;  // --no-output: parse/analyse only, emit nothing
        bool CompileOnly  = false;  // -c: emit the artifact, stop before linking
        bool HeaderGen    = false;
        bool PrintStats   = false;
        bool FtimeReport  = false;

        IntrusiveRefCntPtr<DiagnosticsEngine> CreateDiagnostics(IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts);
        IntrusiveRefCntPtr<DiagnosticOptions> BuildDiagnosticOptions();
        void BuildOptions(FileSystemOptions &FileSystemOpts,
                          std::shared_ptr<TargetOptions> &TargetOpts,
                          FrontendOptions *FrontendOpts,
                          CodeGenOptions *CodeGenOpts);

    public:

        Driver();

        Driver(llvm::ArrayRef<const char *> Args);

        ~Driver();

        CompilerInstance &BuildCompilerInstance();

        void printVersion(bool Full = true);

        bool Execute();

        // Exit code of the suite binary executed by --suite (0 otherwise).
        int getRunExitCode() const { return RunExitCode; }
    };
}

#endif //FLY_DRIVER_H
