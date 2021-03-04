//===--------------------------------------------------------------------------------------------------------------===//
// include/Compiler/CompilerInstance.h - Chain Diagnostic Clients
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_COMPILERINSTANCE_H
#define FLY_COMPILERINSTANCE_H

#include "CompilerInstance.h"
#include "CompilerInvocation.h"
#include "InputFile.h"
#include "FrontendOptions.h"
#include "OutputFile.h"
#include "ChainedDiagnosticConsumer.h"
#include "Basic/FileManager.h"
#include "llvm/Support/VirtualFileSystem.h"

namespace fly {

    class CompilerInstance {

        const InputFile &inputFile;

        FrontendOptions &inputOptions;

        FileManager &fileMgr;

        SourceManager &sourceMgr;

        TargetInfo &target;

        DiagnosticsEngine &diags;

        OutputFile outputFile;

    public:

        CompilerInstance(const CompilerInvocation & invocation,
                         const InputFile & inputFile);

        ~CompilerInstance();

        bool execute();
    };
}


#endif //FLY_COMPILERINSTANCE_H
