//===----------------------------------------------------------------------===//
// include/Frontend/InputOptions.h - Compiler Input options arguments
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//

#ifndef FLY_FRONTENDOPTIONS_H
#define FLY_FRONTENDOPTIONS_H

#include "InputFile.h"
#include "OutputFile.h"
#include <llvm/ADT/ArrayRef.h>
#include <llvm/Option/ArgList.h>
#include <string>

namespace fly {

    class FrontendOptions {

        /// The input files.
        llvm::SmallVector<InputFile, 0> inputFiles;

        OutputFile outputFile;

    public:

        FrontendOptions();

        FrontendOptions(const llvm::SmallVector<InputFile, 0> &inputFiles);

        FrontendOptions(const llvm::SmallVector<InputFile, 0> &inputFiles, const OutputFile &outputFile);

        void addInputFile(InputFile inputFile);

        const llvm::SmallVector<InputFile, 0> &getInputFiles() const;

        const OutputFile &getOutputFile() const;

    };
}


#endif //FLY_FRONTENDOPTIONS_H
