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

#include "InputFile.h"
#include "OutputFile.h"
#include "CodeGen/BackendUtil.h"
#include <string>

namespace fly {

    class FrontendOptions {

        /// The input files.
        llvm::SmallVector<InputFile, 0> Inputs;

        OutputFile Output;

    public:

        /// Generate Library
        bool LibraryGen;

        /// Generate Header
        bool HeaderGen;

        bool Verbose;

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

        void addInputFile(std::string input);

        const llvm::SmallVector<InputFile, 0> &getInputFiles() const;

        const OutputFile &getOutputFile() const;

        void setOutputFile(std::string output, bool isLib = false);

    };
}


#endif //FLY_FRONTENDOPTIONS_H
