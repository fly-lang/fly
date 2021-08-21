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
#include "Frontend/CompilerInstance.h"
#include "Basic/Diagnostic.h"
#include <llvm/Option/Arg.h>
#include <llvm/Option/ArgList.h>

namespace fly {

    class Driver {

        // The Compiler Instance contains components needs for compilation phase
        std::shared_ptr<CompilerInstance> CI;

        // Contains all options before parsing
        const llvm::ArrayRef<const char *> Args;

        // Contains all parsed options
        llvm::opt::InputArgList ArgList;

        // Can go ahead with execute phase
        // It is false if some error happens or an option doesn't allow compilation like help or version
        bool doExecute = true;

        /// The name the driver was invoked as.
        std::string Name;

        /// The path the driver executable was in, as invoked from the
        /// command line.
        std::string Dir;

        /// The original path to the fly executable.
        std::string Path;

        /// The path to the installed fly directory, if any.
        std::string InstalledDir;

        /// The path to the compiler resource directory.
        std::string ResourceDir;

        // Create the diagnostics engine using the invocation's diagnostic options
        // and replace any existing one with it.
        IntrusiveRefCntPtr<DiagnosticsEngine> CreateDiagnostics(IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts);

        IntrusiveRefCntPtr<DiagnosticOptions> BuildDiagnosticOpts();

        void BuildOptions(FileSystemOptions &FileSystemOpts,
                           std::shared_ptr<TargetOptions> &TargetOpts,
                           FrontendOptions *FrontendOpts,
                           CodeGenOptions *CodeGenOpts);

    public:

        Driver();

        Driver(llvm::ArrayRef<const char *> ArgList);

        CompilerInstance &BuildCompilerInstance();

        bool Execute();

        void printVersion(bool full = true);
    };
}

#endif //FLY_DRIVER_H
