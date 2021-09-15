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
#include "Basic/PrettyStackTrace.h"
#include "Basic/FileSystemOptions.h"
#include "Frontend/Frontend.h"
#include "Frontend/ChainedDiagnosticConsumer.h"
#include "Frontend/LogDiagnosticPrinter.h"
#include "CodeGen/BackendUtil.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/CrashRecoveryContext.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/Debug.h"
#include <utility>

using namespace fly;
using namespace fly::driver;

std::string GetExecutablePath(const char *Argv0) {
    SmallString<128> ExecutablePath(Argv0);
    // Do a PATH lookup if Argv0 isn't a valid path.
    if (!llvm::sys::fs::exists(ExecutablePath))
        if (llvm::ErrorOr<std::string> P = llvm::sys::findProgramByName(ExecutablePath))
            ExecutablePath = *P;
    return std::string(ExecutablePath.str());
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

    Name = std::string(llvm::sys::path::filename(Path));
    Dir = std::string(llvm::sys::path::parent_path(Path));
    InstalledDir = Dir; // Provide a sensible default installed dir.
}

CompilerInstance &Driver::BuildCompilerInstance() {

    llvm::PrettyStackTraceString CrashInfo("Building compiler instance");

    // Create diagnostics

    IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = BuildDiagnosticOpts();
    IntrusiveRefCntPtr<DiagnosticsEngine> Diags = CreateDiagnostics(DiagOpts);

    // Create all options.

    FileSystemOptions fileSystemOpts;
    std::shared_ptr<TargetOptions> TargetOpts = std::make_shared<TargetOptions>();
    FrontendOptions *FrontendOpts = new FrontendOptions();
    CodeGenOptions *CodeGenOpts = new CodeGenOptions();
    BuildOptions(fileSystemOpts, TargetOpts, &*FrontendOpts, &*CodeGenOpts);

    CI = std::make_shared<CompilerInstance>(Diags,
                                            std::move(fileSystemOpts),
                                            FrontendOpts,
                                            CodeGenOpts,
                                            std::move(TargetOpts));
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
                            FrontendOptions *FrontendOpts,
                            CodeGenOptions *CodeGenOpts) {

    llvm::PrettyStackTraceString CrashInfo("Command line argument parsing");

    if (Args.empty()) {
        printVersion();
    }

    unsigned MissingArgIndex, MissingArgCount;
    const llvm::opt::OptTable &optTable = fly::driver::getDriverOptTable();
    const unsigned IncludedFlagsBitmask = options::CoreOption;
    ArgList = optTable.ParseArgs(Args, MissingArgIndex, MissingArgCount);

    // Check for missing argument error.
    if (MissingArgCount) {
        llvm::errs() << diag::err_drv_missing_argument << ArgList.getArgString(MissingArgIndex) << MissingArgCount;
        doExecute = false;
        return;
    }

    // Error out on unknown options.
    if (ArgList.hasArg(options::OPT_UNKNOWN)) {
        for (auto *arg : ArgList.filtered(options::OPT_UNKNOWN)) {
            WithColor::error() << "unknown option: " << arg->getSpelling() << '\n';
        }
        llvm::errs() << "Use '" << Path
                     << " --help' for a complete list of options.\n";
        doExecute = false;
        return;
    }

    // Show Version
    if (ArgList.hasArg(options::OPT_VERSION)) {
        printVersion();
        doExecute = false;
        return;
    }
    if (ArgList.hasArg(options::OPT_VERSION_SHORT)) {
        printVersion(false);
        doExecute = false;
        return;
    }

    // Show Help
    if (ArgList.hasArg(options::OPT_HELP)) {
        getDriverOptTable().PrintHelp(
                llvm::outs(), "fly [options] source.fly ...\n",
                "Example: fly -v -o out main.fly\n"
                "Fly Compiler",
                /*Include=*/driver::options::CoreOption, /*Exclude=*/0,
                /*ShowAllAliases=*/false);
        doExecute = false;
        return;
    }

    // Parse Input args
    if (ArgList.hasArg(options::OPT_INPUT)) {
        for (const llvm::opt::Arg *a : ArgList.filtered(options::OPT_INPUT)) {
            FrontendOpts->addInputFile(a->getValue());
        }
    }

    // Enable Verbose Log
    if (ArgList.hasArg(options::OPT_VERBOSE)) {
        FrontendOpts->Verbose = true;
    }

    // Output Options

    // Parse Output arg
    if (ArgList.hasArg(options::OPT_OUTPUT)) {
        const StringRef &output = ArgList.getLastArgValue(options::OPT_OUTPUT);
        FrontendOpts->setOutputFile(output);
    }
    // Set Working Directory
    if (const llvm::opt::Arg *A = ArgList.getLastArg(options::OPT_WORKING_DIR))
        fileSystemOpts.WorkingDir = A->getValue();

    // Stats
    FrontendOpts->ShowStats = ArgList.hasArg(options::OPT_PRINT_STATS);
    FrontendOpts->ShowTimers = ArgList.hasArg(options::OPT_FTIME_REPORT);
    FrontendOpts->StatsFile = std::string(ArgList.getLastArgValue(options::OPT_STATS_FILE));

    if (ArgList.hasArg(options::OPT_DEBUG)) {
        llvm::DebugFlag = true;
    }

    // Emit different kind of file
    if (ArgList.hasArg(options::OPT_EMIT_LL)) {
        FrontendOpts->BackendAction = BackendActionKind::Backend_EmitLL;
    } else if (ArgList.hasArg(options::OPT_EMIT_BC)) {
        FrontendOpts->BackendAction = BackendActionKind::Backend_EmitBC;
    } else if (ArgList.hasArg(options::OPT_EMIT_AS)) {
        FrontendOpts->BackendAction =BackendActionKind::Backend_EmitAssembly;
    } else if (ArgList.hasArg(options::OPT_EMIT_NOTHING)) {
        FrontendOpts->BackendAction = BackendActionKind::Backend_EmitNothing;
    } else {
        FrontendOpts->BackendAction = BackendActionKind::Backend_EmitObj;
    }
    // Target Options
    if (const llvm::opt::Arg *A = ArgList.getLastArg(options::OPT_TARGET))
        targetOpts->Triple = A->getValue();
    else
        targetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
    targetOpts->CodeModel = std::string(ArgList.getLastArgValue(options::OPT_MC_MODEL, "default"));
    targetOpts->CPU = std::string(ArgList.getLastArgValue(options::OPT_TARGET_CPU));

    // CodeGen Options
    CodeGenOpts->CodeModel = targetOpts->CodeModel;
    CodeGenOpts->ThreadModel = std::string(ArgList.getLastArgValue(options::OPT_MTHREAD_MODEL, "posix"));
    if (CodeGenOpts->ThreadModel != "posix" && CodeGenOpts->ThreadModel != "single")
        llvm::errs() << ArgList.getLastArg(options::OPT_MTHREAD_MODEL)->getAsString(ArgList)
                << CodeGenOpts->ThreadModel;
}

bool Driver::Execute() {
    bool Success = true;

    if (doExecute) {

        Frontend Front(*CI);
        Success = Front.Execute();
    }
    return Success;
}

void Driver::printVersion(bool full) {
    if (full) {
        llvm::outs() << "FLY version " << FLY_VERSION << " (https://flylang.org)" << "\n";
        llvm::outs() << "with LLVM version " << LLVM_VERSION_STRING << "\n";
    } else {
        llvm::outs() << FLY_VERSION << "\n";
    }
}
