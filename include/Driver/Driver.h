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
        std::vector<std::string> InputFiles;
        std::vector<std::string> LibDirs;
        std::vector<std::string> LinkLibs;  // --link-lib flags: external C libs to link (-lNAME)
        std::string OutputFile;
        bool OutputLib    = false;
        bool OutputShared = false;
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
        bool Verbose      = false;
        bool NoWarnings   = false;
        bool EmitLL       = false;
        bool EmitBC       = false;
        bool EmitAS       = false;
        bool NoOutput     = false;
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
    };
}

#endif //FLY_DRIVER_H
