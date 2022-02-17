//===--------------------------------------------------------------------------------------------------------------===//
// include/Frontend/InputOptions.h - Compiler Input options arguments
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

        bool OutputLib;

    public:

        /// Generate Library
        bool CreateLibrary;

        /// Generate Header
        bool CreateHeader;

        /// Enable Verbose output
        bool Verbose;

        /// The Backend action
        BackendActionKind BackendAction;

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
