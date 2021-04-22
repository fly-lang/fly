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
#include <llvm/Option/Arg.h>
#include <llvm/Option/ArgList.h>
#include <Frontend/CompilerInstance.h>

namespace fly {

    class Driver {

        std::shared_ptr<CompilerInstance> CI;

        llvm::opt::InputArgList ArgList;

        llvm::ArrayRef<const char *> Args;

        bool CanExecute;

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
        IntrusiveRefCntPtr<DiagnosticsEngine> CreateDiagnostics(IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts);

        IntrusiveRefCntPtr<DiagnosticOptions> BuildDiagnosticOpts();

        void BuildOptions(FileSystemOptions &FileSystemOpts,
                           std::shared_ptr<TargetOptions> &TargetOpts,
                           std::unique_ptr<FrontendOptions> &FrontendOpts,
                           std::unique_ptr<CodeGenOptions> &CodeGenOpts);

        /// The name the driver was invoked as.
        llvm::StringRef Name;

        /// The path the driver executable was in, as invoked from the
        /// command line.
        llvm::StringRef Dir;

        /// The original path to the fly executable.
        llvm::StringRef Path;

        /// The path to the installed fly directory, if any.
        llvm::StringRef InstalledDir;

        /// The path to the compiler resource directory.
        llvm::StringRef ResourceDir;

    public:

        Driver();

        Driver(llvm::ArrayRef<const char *> ArgList);

        CompilerInstance &BuildCompilerInstance();

        bool Execute();
    };
}

#endif //FLY_DRIVER_H
