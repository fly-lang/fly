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

#include "CompilerInstance.h"
#include "CompilerInvocation.h"
#include "FrontendOptions.h"
#include "Basic/Diagnostic.h"

namespace fly {
    using namespace llvm;

    class CompilerInstance;
    class CompilerInvocation;

    class Frontend {

        /// The diagnostics engine instance.
        DiagnosticsEngine &diagnostics;

        // Compiler Invocation contains all a CompilerInstance needs
        CompilerInvocation &invocation;

        // Compiler instances
        std::vector<CompilerInstance*> instances;

    public:

        ~Frontend();

        explicit Frontend(CompilerInvocation &invocation);

        bool execute() const;

        CompilerInvocation &getInvocation();

        const std::vector<CompilerInstance *> &getInstances() const;
    };
}


#endif //FLY_FRONTEND_H
