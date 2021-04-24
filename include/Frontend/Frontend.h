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

namespace fly {
    using namespace llvm;

    class FrontendAction;
    class CompilerInstance;

    class Frontend {

        /// The diagnostics engine instance.
        DiagnosticsEngine &Diags;

        // Compiler Invocation contains all a CompilerInstance needs
        CompilerInstance &CI;

        // Compiler instances
        std::vector<FrontendAction*> Actions;

        ASTContext* Context;

    public:

        explicit Frontend(CompilerInstance &CI);

        ~Frontend();

        bool Execute() const;
        
        const std::vector<FrontendAction *> &getActions() const;
    };
}


#endif //FLY_FRONTEND_H
