//===----------------------------------------------------------------------===//
// Compiler/Compiler.h - Compiler Main
//
// Part of the Fly Project, under the Apache License v2.0
// See https://flylang.org/LICENSE.txt for license information.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//

#ifndef FLY_COMPILER_H
#define FLY_COMPILER_H

#include "CompilerInstance.h"
#include "CompilerInvocation.h"
#include "InputOptions.h"
#include "Basic/Diagnostic.h"

namespace fly {
    using namespace llvm;

    class CompilerInstance;
    class CompilerInvocation;

    class Compiler {

        /// The diagnostics engine instance.
        IntrusiveRefCntPtr<DiagnosticsEngine> diagnostics;

        // Compiler Invocation contains all a CompilerInstance needs
        CompilerInvocation *invocation;

        // Compiler instances
        std::vector<CompilerInstance*> instances;

        /// Create the diagnostics engine using the invocation's diagnostic options
        /// and replace any existing one with it.
        ///
        /// Note that this routine also replaces the diagnostic client,
        /// allocating one if one is not provided.
        ///
        /// \param client If non-NULL, a diagnostic client that will be
        /// attached to (and, then, owned by) the DiagnosticsEngine inside this AST
        /// unit.
        ///
        /// \param shouldOwnClient If Client is non-NULL, specifies whether
        /// the diagnostic object should take ownership of the client.
        void createDiagnostics(DiagnosticConsumer *client = nullptr,
                               bool shouldOwnClient = true);

    public:

        ~Compiler();

        Compiler(const std::string& path, ArrayRef<const char *> argList);

        Compiler(int argc, const char **argv);

        bool execute() const;

        /// Get the current diagnostics engine.
        DiagnosticsEngine &getDiagnostics() const {
            assert(diagnostics && "Compiler instance has no diagnostics!");
            return *diagnostics;
        }

        static std::string GetExecutablePath(const char *argv0, void *mainAddr);

        static std::string getPath(const char *first);

        static SmallVector<const char *, 16> getArgs(int argc, const char **argv);

        CompilerInvocation &getInvocation();

        const std::vector<CompilerInstance *> &getInstances() const;
    };
}


#endif //FLY_COMPILER_H
