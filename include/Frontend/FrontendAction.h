//===--------------------------------------------------------------------------------------------------------------===//
// include/Compiler/CompilerInstance.h - Chain Diagnostic Clients
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_FRONTENDACTION_H
#define FLY_FRONTENDACTION_H

#include "FrontendAction.h"
#include "CompilerInstance.h"
#include "InputFile.h"
#include "FrontendOptions.h"
#include "OutputFile.h"
#include "ChainedDiagnosticConsumer.h"
#include "Basic/FileManager.h"
#include "llvm/Support/VirtualFileSystem.h"

namespace fly {

    class FrontendAction {

        const InputFile &Input;

        FrontendOptions &FrontendOpts;

        CodeGenOptions &CodeGenOpts;

        TargetOptions TargetOpts;

        FileManager &FileMgr;

        SourceManager &SourceMgr;

        TargetInfo &Target;

        DiagnosticsEngine &Diags;

        OutputFile Output;

    public:

        FrontendAction(const CompilerInstance & CI, const InputFile & Input);

        ~FrontendAction();

        bool Execute();
    };
}


#endif //FLY_FRONTENDACTION_H
