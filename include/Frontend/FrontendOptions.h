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
#include <llvm/ADT/ArrayRef.h>
#include <llvm/Option/ArgList.h>
#include <string>

namespace fly {

    class FrontendOptions {

        /// The input files.
        llvm::SmallVector<InputFile, 0> Inputs;

        OutputFile Output;

    public:

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

        void addInputFile(llvm::StringRef input);

        const llvm::SmallVector<InputFile, 0> &getInputFiles() const;

        const OutputFile &getOutputFile() const;

        void setOutputFile(llvm::StringRef output);

        bool isVerbose() const;

        void setVerbose();

        BackendActionKind getBackendAction();

        void setBackendAction(BackendActionKind action);

    };
}


#endif //FLY_FRONTENDOPTIONS_H
