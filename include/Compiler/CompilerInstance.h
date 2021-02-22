//===----------------------------------------------------------------------===//
// Compiler/CompilerInstance.h - Chain Diagnostic Clients
//
// Part of the Fly Project, under the Apache License v2.0
// See https://flylang.org/LICENSE.txt for license information.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//

#ifndef FLY_COMPILERINSTANCE_H
#define FLY_COMPILERINSTANCE_H

#include "CompilerInstance.h"
#include "CompilerInvocation.h"
#include "InputFile.h"
#include "InputOptions.h"
#include "OutputFile.h"
#include "ChainedDiagnosticConsumer.h"
#include "Basic/FileManager.h"
#include "llvm/Support/VirtualFileSystem.h"

namespace fly {

    class CompilerInstance {

        const InputFile &inputFile;

        const InputOptions &inputOptions;

        FileManager &fileMgr;

        SourceManager &sourceMgr;

        DiagnosticsEngine &diagnostics;

        OutputFile outputFile;

    public:

        CompilerInstance(const CompilerInvocation & invocation,
                         const InputFile & inputFile);

        ~CompilerInstance();

        bool execute();
    };
}


#endif //FLY_COMPILERINSTANCE_H
