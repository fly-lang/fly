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

        bool Verbose;

        BackendAction Action;

    public:

        void addInputFile(llvm::StringRef input);

        const llvm::SmallVector<InputFile, 0> &getInputFiles() const;

        const OutputFile &getOutputFile() const;

        void setOutputFile(const char * output);

        bool isVerbose() const;

        void setVerbose();

        BackendAction getBackendAction();

        void setBackendAction(BackendAction action);

    };
}


#endif //FLY_FRONTENDOPTIONS_H
