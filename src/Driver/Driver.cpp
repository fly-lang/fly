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
#include "Basic/PrettyStackTrace.h"
#include "Basic/FileSystemOptions.h"
#include "Frontend/Frontend.h"
#include "Frontend/ChainedDiagnosticConsumer.h"
#include "Frontend/LogDiagnosticPrinter.h"
#include "CodeGen/BackendUtil.h"
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/CrashRecoveryContext.h>
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Timer.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Program.h"
#include <utility>

using namespace fly;
using namespace fly::driver;

llvm::StringRef GetExecutablePath(const char *Argv0) {
    SmallString<128> ExecutablePath(Argv0);
    // Do a PATH lookup if Argv0 isn't a valid path.
    if (!llvm::sys::fs::exists(ExecutablePath))
        if (llvm::ErrorOr<std::string> P = llvm::sys::findProgramByName(ExecutablePath))
            ExecutablePath = *P;
    return ExecutablePath;
}

SmallVector<const char *, 256> initDriver() {
    int Argc = 1;
    const char *Argv[] = {"fly"};
    SmallVector<const char *, 256> Args(Argv, Argv + Argc);
    return Args;
}

Driver::Driver() : Driver(initDriver()) {}

Driver::Driver(llvm::ArrayRef<const char *> ArrArgs) :
        Path(GetExecutablePath(ArrArgs[0])),
        Args(ArrArgs.slice(1)) {

    Name = llvm::sys::path::filename(Path);
    Dir = llvm::sys::path::parent_path(Path);
    InstalledDir = Dir; // Provide a sensible default installed dir.
}

CompilerInstance &Driver::BuildCompilerInstance() {

    llvm::PrettyStackTraceString CrashInfo("Building compiler instance");

    // Create diagnostics

    IntrusiveRefCntPtr <DiagnosticOptions> DiagOpts = BuildDiagnosticOpts();
    IntrusiveRefCntPtr<DiagnosticsEngine> Diags = CreateDiagnostics(DiagOpts);

    // Create all options.

    FileSystemOptions fileSystemOpts;
    std::shared_ptr<TargetOptions> targetOpts = std::make_shared<TargetOptions>();
    std::unique_ptr<FrontendOptions> frontendOpts = std::make_unique<FrontendOptions>();
    std::unique_ptr<CodeGenOptions> codeGenOpts = std::make_unique<CodeGenOptions>();
    BuildOptions(fileSystemOpts, targetOpts, frontendOpts, codeGenOpts);

    CI = std::make_shared<CompilerInstance>(Diags,
                                            std::move(fileSystemOpts),
                                            std::move(frontendOpts),
                                            std::move(codeGenOpts),
                                            std::move(targetOpts));
    if (!CI) {
        llvm::errs() << "Error while creating compiler instance!" << "\n";
        exit(1);
    }

    return *CI;
}

// Diagnostics

IntrusiveRefCntPtr<DiagnosticsEngine> Driver::CreateDiagnostics(IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts) {
    DiagOpts = new DiagnosticOptions;
    TextDiagnosticPrinter *DiagClient = new TextDiagnosticPrinter(llvm::errs(), &*DiagOpts);
    StringRef ExeBasename(llvm::sys::path::stem(Path));
    DiagClient->setPrefix(std::string(ExeBasename));
    IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
    IntrusiveRefCntPtr<DiagnosticsEngine> Diags = new DiagnosticsEngine(DiagID, &*DiagOpts, DiagClient);

    raw_ostream *OS = &llvm::errs();
    std::error_code EC;
    std::unique_ptr<raw_ostream> StreamOwner;

    if (!DiagOpts->DiagnosticLogFile.empty()) {
        // Create the output stream.
        auto FileOS = std::make_unique<llvm::raw_fd_ostream>(
                DiagOpts->DiagnosticLogFile, EC,
                llvm::sys::fs::OF_Append | llvm::sys::fs::OF_Text);
        if (EC) {
            Diags->Report(diag::warn_fe_cc_log_diagnostics_failure)
                    << DiagOpts->DiagnosticLogFile << EC.message();
        } else {
            FileOS->SetUnbuffered();
            OS = FileOS.get();
            StreamOwner = std::move(FileOS);
        }
    }

    // Chain in the diagnostic client which will log the diagnostics.
    auto Logger = std::make_unique<LogDiagnosticPrinter>(*OS, DiagOpts.get(), std::move(StreamOwner));

    if (Diags->ownsClient()) {
        Diags->setClient(
                new ChainedDiagnosticConsumer(Diags->takeClient(), std::move(Logger)));
    } else {
        Diags->setClient(
                new ChainedDiagnosticConsumer(Diags->getClient(), std::move(Logger)));
    }


    ProcessWarningOptions(*Diags, *DiagOpts, /*ReportDiags=*/false);

    return std::move(Diags);
}

IntrusiveRefCntPtr<DiagnosticOptions> Driver::BuildDiagnosticOpts() {
    IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts(new DiagnosticOptions);
    DiagOpts->DiagnosticLogFile = std::string(ArgList.getLastArgValue(options::OPT_LOG_FILE));
    DiagOpts->IgnoreWarnings = ArgList.hasArg(options::OPT_NO_WARNING);
    return std::move(DiagOpts);
}

void Driver::BuildOptions(FileSystemOptions &fileSystemOpts,
                            std::shared_ptr<TargetOptions> &targetOpts,
                            std::unique_ptr<FrontendOptions> &frontendOpts,
                            std::unique_ptr<CodeGenOptions> &codeGenOpts) {

    llvm::PrettyStackTraceString CrashInfo("Command line argument parsing");

    unsigned MissingArgIndex, MissingArgCount;
    const llvm::opt::OptTable &optTable = fly::driver::getDriverOptTable();
    const unsigned IncludedFlagsBitmask = options::CoreOption;
    ArgList = optTable.ParseArgs(Args, MissingArgIndex, MissingArgCount);

    // Check for missing argument error.
    if (MissingArgCount) {
        llvm::errs() << diag::err_drv_missing_argument << ArgList.getArgString(MissingArgIndex) << MissingArgCount;
        CanExecute = false;
        return;
    }

    // Error out on unknown options.
    if (ArgList.hasArg(options::OPT_UNKNOWN)) {
        for (auto *arg : ArgList.filtered(options::OPT_UNKNOWN)) {
            WithColor::error() << "unknown option: " << arg->getSpelling() << '\n';
        }
        llvm::errs() << "Use '" << Path
                     << " --help' for a complete list of options.\n";
        CanExecute = false;
        return;
    }

    // Show Version
    if (ArgList.hasArg(options::OPT_VERSION)) {
        llvm::outs() << "FLY version " << FLY_VERSION << " (https://flylang.org)" << "\n"
                     << "with LLVM version " << LLVM_VERSION_STRING << "\n";
        CanExecute = false;
        return;
    }

    // Show Help
    if (ArgList.hasArg(options::OPT_HELP)) {
        getDriverOptTable().PrintHelp(
                llvm::outs(), "fly [options] file...",
                "Fly Compiler",
                /*Include=*/driver::options::CoreOption, /*Exclude=*/0,
                /*ShowAllAliases=*/false);
        CanExecute = false;
        return;
    }

    // Parse Input args
    if (ArgList.hasArg(options::OPT_INPUT)) {
        for (const llvm::opt::Arg *a : ArgList.filtered(options::OPT_INPUT)) {
            frontendOpts->addInputFile(a->getValue());
        }
    }

    // Parse Output arg
    if (ArgList.hasArg(options::OPT_OUTPUT)) {
        const StringRef &output = ArgList.getLastArgValue(options::OPT_OUTPUT);
        frontendOpts->setOutputFile(output);
    }

    if (ArgList.hasArg(options::OPT_VERBOSE)) {
        frontendOpts->setVerbose();
    }

    if (ArgList.hasArg(options::OPT_EMIT_LL)) {
        frontendOpts->setBackendAction(BackendAction::Backend_EmitLL);
    } else if (ArgList.hasArg(options::OPT_EMIT_BC)) {
        frontendOpts->setBackendAction(BackendAction::Backend_EmitBC);
    } else if (ArgList.hasArg(options::OPT_EMIT_AS)) {
        frontendOpts->setBackendAction(BackendAction::Backend_EmitAssembly);
    } else if (ArgList.hasArg(options::OPT_EMIT_NOTHING)) {
        frontendOpts->setBackendAction(BackendAction::Backend_EmitNothing);
    } else {
        frontendOpts->setBackendAction(BackendAction::Backend_EmitObj);
    }

    if (const llvm::opt::Arg *A = ArgList.getLastArg(options::OPT_WORKING_DIR))
        fileSystemOpts.WorkingDir = A->getValue();

    if (const llvm::opt::Arg *A = ArgList.getLastArg(options::OPT_TARGET))
        targetOpts->Triple = A->getValue();
    else
        targetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
}

bool Driver::Execute() {
    bool Success = true;

    if (CanExecute) {

        Frontend Front(*CI);
        Success = Front.Execute();

        CI->getDiagnostics().getClient()->finish();
    }

    return Success;
}
