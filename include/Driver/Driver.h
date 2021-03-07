//===--------------------------------------------------------------------------------------------------------------===//
// include/Driver/Driver.h - Driver
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_DRIVER_H
#define FLY_DRIVER_H

#include "Driver/DriverOptions.h"
#include "Frontend/FrontendOptions.h"
#include "Basic/Diagnostic.h"
#include <llvm/Option/ArgList.h>
#include <Frontend/CompilerInvocation.h>

namespace fly {
    class Driver {

        std::shared_ptr<CompilerInvocation> invocation;

    public:

        /// The name the driver was invoked as.
        std::string name;

        /// The path the driver executable was in, as invoked from the
        /// command line.
        std::string dir;

        /// The original path to the clang executable.
        std::string executable;

        /// The path to the installed clang directory, if any.
        std::string installedDir;

        /// The path to the compiler resource directory.
        std::string resourceDir;

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
        IntrusiveRefCntPtr<DiagnosticsEngine> createDiagnostics();

        bool CreateFromArgs(DiagnosticsEngine &diags, llvm::ArrayRef<const char *> ArgStrings,
                            std::unique_ptr<FrontendOptions> &frontendOpts,
                            std::unique_ptr<CodeGenOptions> &codegenOpts);

        IntrusiveRefCntPtr<TargetInfo> createTargetInfo(DiagnosticsEngine &diags);

        const std::shared_ptr<CompilerInvocation> &getInvocation() const;

        bool execute();
    };
}

#endif //FLY_DRIVER_H
