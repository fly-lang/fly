//===--------------------------------------------------------------------------------------------------------------===//
// include/Frontend/FrontendOptions.h - frontend options
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_FRONTENDOPTIONS_H
#define FLY_FRONTENDOPTIONS_H

#include "CodeGen/BackendUtil.h"
#include <string>

namespace fly {

    class FrontendOptions {

        /// The input files.
        llvm::SmallVector<std::string, 16> Inputs;

        std::string Output;

        bool OutputLib = false;

    public:

        /// Stdlib directory — loaded as .fly source (preferDotFlyH=false).
        /// Discovered at runtime as <fly_bin>/../share/lib.
        std::string StdLibDir;

        /// Additional library search directories (added via -L <dir>).
        /// Each directory is scanned for .fly.h headers (preferDotFlyH=true).
        llvm::SmallVector<std::string, 4> LibDirs;

        /// Source search dirs added via --src-dir. Scanned for .fly files whose
        /// namespace matches an import when building the dependency graph.
        llvm::SmallVector<std::string, 4> SrcDirs;

        /// True when a single input file was passed without --lib/--shared:
        /// the output type is inferred from the parsed AST (main/suite/lib).
        bool AutoDetectOutput = false;

        /// Generate Library
        bool CreateLibrary = false;

        /// Generate Shared Library (.so/.dylib/.dll)
        bool CreateSharedLib = false;

        /// Generate Header
        bool CreateHeader = false;

        /// Enable Verbose output
        bool Verbose = false;

        /// The Backend action
        BackendActionKind BackendAction = Backend_EmitObj;

        /// Show the -version text.
        bool ShowVersion = false;

        /// Show the -help text.
        bool ShowHelp = false;

        /// Show frontend performance metrics and statistics.
        bool ShowStats = false;

        /// Show timers for individual actions.
        bool ShowTimers = false;

        /// Filename to write statistics to.
        std::string StatsFile;

        void addInputFile(const char* FileName);

        const llvm::SmallVector<std::string, 16> &getInputFiles() const;

        const std::string &getOutputFile() const;

        void setOutputFile(llvm::StringRef FileName, bool isLib = false);

        bool isOutputLib() const;

    };
}


#endif //FLY_FRONTENDOPTIONS_H
