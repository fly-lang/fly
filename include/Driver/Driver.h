//
// Created by marco on 2/27/21.
//

#ifndef FLY_DRIVER_H
#define FLY_DRIVER_H

#include "Driver/DriverOptions.h"
#include "Frontend/FrontendOptions.h"
#include "Basic/Diagnostic.h"
#include <llvm/Option/ArgList.h>
#include <Frontend/CompilerInvocation.h>

namespace fly {
    class Driver {

        const std::string &path;

        llvm::opt::InputArgList argList;

        IntrusiveRefCntPtr<DiagnosticsEngine> diagnostics;

        std::shared_ptr<CompilerInvocation> invocation;

    public:

        Driver();

        Driver(int argc, const char **argv);

        Driver(const std::string &path, llvm::ArrayRef<const char *> argList);

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

        void createInvocation();

        bool execute();

        const std::string &getPath() const;

        const llvm::opt::InputArgList &getArgList() const;

        const IntrusiveRefCntPtr<DiagnosticsEngine> &getDiagnostics() const;

        const std::shared_ptr<CompilerInvocation> &getInvocation() const;
    };
}

#endif //FLY_DRIVER_H
