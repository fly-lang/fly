//===--------------------------------------------------------------------------------------------------------------===//
// src/Driver/Driver.cpp - Driver
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Driver/Driver.h"
#include "Driver/DriverOptions.h"
#include "Config/config.h"
#include "Basic/DiagnosticDriver.h"
#include "Frontend/Frontend.h"
#include "Frontend/ChainedDiagnosticConsumer.h"
#include "Frontend/LogDiagnosticPrinter.h"
#include <llvm/Support/FileSystem.h>
#include <Basic/PrettyStackTrace.h>
#include <llvm/Support/CommandLine.h>

using namespace fly;
using namespace fly::driver;

// This function isn't referenced outside its translation unit, but it
// can't use the "static" keyword because its address is used for
// GetMainExecutable (since some platforms don't support taking the
// address of main, and some platforms can't implement GetMainExecutable
// without being given the address of a function in the main executable).
std::string GetExecutablePath(const char *argv0, void *mainAddr) {
    return llvm::sys::fs::getMainExecutable(argv0, mainAddr);
}

std::string getCurPath(const char *argv0) {
    // This just needs to be some symbol in the binary; C++ doesn't
    // allow taking the address of ::main however.
    void *mainAddr = (void *) (intptr_t) GetExecutablePath;
    return GetExecutablePath(argv0, mainAddr);
}

llvm::SmallVector<const char *, 16> getArgs(int argc, const char **argv) {
    llvm::SmallVector<const char *, 16> args(argv, argv + argc);
    return std::move(args);
}

void SetUpDiagnosticLog(DiagnosticOptions *diagOpts,
                               DiagnosticsEngine &diags) {
    std::error_code EC;
    std::unique_ptr<raw_ostream> StreamOwner;
    raw_ostream *OS = &llvm::errs();
    if (diagOpts->DiagnosticLogFile != "-") {
        // Create the output stream.
        auto FileOS = std::make_unique<llvm::raw_fd_ostream>(
                diagOpts->DiagnosticLogFile, EC,
                llvm::sys::fs::OF_Append | llvm::sys::fs::OF_Text);
        if (EC) {
            diags.Report(diag::warn_fe_cc_log_diagnostics_failure)
                    << diagOpts->DiagnosticLogFile << EC.message();
        } else {
            FileOS->SetUnbuffered();
            OS = FileOS.get();
            StreamOwner = std::move(FileOS);
        }
    }

    // Chain in the diagnostic client which will log the diagnostics.
    auto Logger = std::make_unique<LogDiagnosticPrinter>(*OS, diagOpts, std::move(StreamOwner));

    if (diags.ownsClient()) {
        diags.setClient(
                new ChainedDiagnosticConsumer(diags.takeClient(), std::move(Logger)));
    } else {
        diags.setClient(
                new ChainedDiagnosticConsumer(diags.getClient(), std::move(Logger)));
    }
}

Driver::Driver() : Driver("fly", {"fly"}) {}

Driver::Driver(int argc, const char **argv) :
    Driver(getCurPath(argv[0]), ((ArrayRef<const char*>)getArgs(argc, argv)).slice(1)) {}

Driver::Driver(const std::string& path, llvm::ArrayRef<const char *> args) : executable(path) {

    name = llvm::sys::path::filename(path);
    dir = llvm::sys::path::parent_path(path);
    installedDir = dir; // Provide a sensible default installed dir.

    // Create diagnostics
    IntrusiveRefCntPtr<DiagnosticsEngine> diags = createDiagnostics();

    // Create target info
    IntrusiveRefCntPtr<TargetInfo> target = createTargetInfo(*diags);

    std::unique_ptr<FrontendOptions> frontendOpts = std::make_unique<FrontendOptions>();
    std::unique_ptr<CodeGenOptions> codeGenOpts = std::make_unique<CodeGenOptions>();
    if (CreateFromArgs(*diags, args, frontendOpts, codeGenOpts)) {
        invocation = std::make_shared<CompilerInvocation>(std::move(diags), std::move(target),
                                                          std::move(frontendOpts), std::move(codeGenOpts));
    }
}

// Diagnostics

IntrusiveRefCntPtr<DiagnosticsEngine> Driver::createDiagnostics(DiagnosticConsumer *client, bool shouldOwnClient) {
    DiagnosticOptions *DiagOpts = new DiagnosticOptions();
    IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
    IntrusiveRefCntPtr<DiagnosticsEngine> diagnostics = new DiagnosticsEngine(DiagID, DiagOpts);

    // Create the diagnostic client for reporting errors or for
    // implementing -verify.
    if (client) {
        diagnostics->setClient(client, shouldOwnClient);
    } else
        diagnostics->setClient(new TextDiagnosticPrinter(llvm::errs(), DiagOpts));

    // Chain in -diagnostic-log-file dumper, if requested.
    if (!DiagOpts->DiagnosticLogFile.empty())
        SetUpDiagnosticLog(DiagOpts, *diagnostics);

    // Configure our handling of diagnostics.
    ProcessWarningOptions(*diagnostics, *DiagOpts);
    
    return std::move(diagnostics);
}

bool Driver::CreateFromArgs(DiagnosticsEngine &diags,
                            llvm::ArrayRef<const char *> ArgStrings,
                            std::unique_ptr<FrontendOptions> &frontendOpts,
                            std::unique_ptr<CodeGenOptions> &codegenOpts) {
    llvm::PrettyStackTraceString CrashInfo("Command line argument parsing");

    unsigned MissingArgIndex, MissingArgCount;
    const llvm::opt::OptTable &optTable = fly::driver::getDriverOptTable();
    const unsigned IncludedFlagsBitmask = options::CoreOption;
    llvm::opt::InputArgList argList = optTable.ParseArgs(ArgStrings, MissingArgIndex,MissingArgCount);

    bool success = true;

    // Check for missing argument error.
    if (MissingArgCount) {
        diags.Report(diag::err_drv_missing_argument)
                << argList.getArgString(MissingArgIndex) << MissingArgCount;
        success = false;
    }

    // Issue errors on unknown arguments.
    for (const llvm::opt::Arg *A : argList.filtered(options::OPT_UNKNOWN)) {
        auto argString = A->getAsString(argList);
        std::string Nearest;
        if (optTable.findNearest(argString, Nearest, IncludedFlagsBitmask) > 1)
            diags.Report(diag::err_drv_unknown_argument) << argString;
        else
            diags.Report(diag::err_drv_unknown_argument_with_suggestion)
                    << argString << Nearest;
        success = false;
    }

    // Show Version
    if (argList.hasArg(options::OPT_VERSION)) {
        llvm::outs() << "FLY version " << FLY_VERSION << " (https://flylang.org)" << "\n"
                     << "with LLVM version " << LLVM_VERSION_STRING << "\n";
        return false;
    }

    // Show Help
    if (argList.hasArg(options::OPT_HELP)) {
        getDriverOptTable().PrintHelp(
                llvm::outs(), "fly [options] file...",
                "Fly Compiler",
                /*Include=*/driver::options::CoreOption, /*Exclude=*/0,
                /*ShowAllAliases=*/false);
        return false;
    }

    // Parse Input args
    if (argList.hasArg(options::OPT_INPUT)) {
        bool First = true;
        for (const llvm::opt::Arg *a : argList.filtered(options::OPT_INPUT)) {
            if (First) {
                frontendOpts->addInputFile(a->getValue());
                First = false;
            } else {
                diags.Report(diag::err_drv_unknown_argument) << a->getAsString(argList);
                success = false;
            }
        }
    }

    // Parse Output arg
    if (argList.hasArg(options::OPT_OUTPUT)) {
        const StringRef &output = argList.getLastArgValue(options::OPT_OUTPUT);
        frontendOpts->setOutputFile(output.str().c_str());
    }

    if (argList.hasArg(options::OPT_VERBOSE)) {
        frontendOpts->setVerbose();
    }

    return success;
}

IntrusiveRefCntPtr<TargetInfo> Driver::createTargetInfo(DiagnosticsEngine &diags) {
    std::shared_ptr<TargetOptions> targetOpts(new TargetOptions);
    targetOpts->Triple = llvm::sys::getProcessTriple();
    return TargetInfo::CreateTargetInfo(diags, targetOpts);
}

const std::shared_ptr<CompilerInvocation> &Driver::getInvocation() const {
    return invocation;
}

bool Driver::execute() {
    // Single Job
    if (invocation) {
        Frontend frontend(*invocation);
        return frontend.execute();
    }
    return false;
}
