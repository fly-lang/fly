//===--------------------------------------------------------------------------------------------------------------===//
// include/Frontend/Frontend.h - Frontends Main
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_FRONTEND_H
#define FLY_FRONTEND_H

#include "FrontendAction.h"
#include "CompilerInstance.h"
#include "FrontendOptions.h"
#include "Basic/Diagnostic.h"
#include "AST/ASTContext.h"

namespace llvm {
    class TimerGroup;
    class Timer;
}

namespace fly {

    class FrontendAction;
    class CompilerInstance;
    class SemaBuilder;
    class Sema;

    class Frontend {

        // The diagnostics engine instance.
        DiagnosticsEngine &Diags;

        // Compiler Invocation contains all a CompilerInstance needs
        CompilerInstance &CI;

        llvm::SmallVector<std::string, 4> OutputFiles;

        /// The frontend timer group.
        std::unique_ptr<llvm::TimerGroup> FrontendTimerGroup;

        /// The frontend timer.
        std::unique_ptr<llvm::Timer> FrontendTimer;

    public:

        explicit Frontend(CompilerInstance &CI);

        ~Frontend();

        bool Execute();

        std::vector<FrontendAction *> ParseActions(CodeGen &CG, Sema &S);

        void CreateFrontendTimer();

        const SmallVector<std::string, 4> &getOutputFiles() const;

        std::vector<StringRef> ExtractFiles(const std::string &LibFileName);
    };
}


#endif //FLY_FRONTEND_H
