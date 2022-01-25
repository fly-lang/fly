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
#include "Driver/ToolChain.h"
#include "Config/Config.h"
#include "Basic/PrettyStackTrace.h"
#include "Basic/FileSystemOptions.h"
#include "Frontend/Frontend.h"
#include "Frontend/ChainedDiagnosticConsumer.h"
#include "Frontend/LogDiagnosticPrinter.h"
#include "CodeGen/BackendUtil.h"
#include "Basic/Debug.h"
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

    unsigned MissingArgIndex, MissingArgCount;
    const llvm::opt::OptTable &optTable = fly::driver::getDriverOptTable();
    const unsigned IncludedFlagsBitmask = options::CoreOption;
    ArgList = optTable.ParseArgs(Args, MissingArgIndex, MissingArgCount);

    if (ArgList.hasArg(options::OPT_DEBUG)) {
        llvm::DebugFlag = true;
        FLY_DEBUG_MESSAGE("Driver", "Driver", "Set OPT_DEBUG");
    }
}

Driver::~Driver() {

}

CompilerInstance &Driver::BuildCompilerInstance() {
    FLY_DEBUG("Driver", "BuildCompilerInstance");
    llvm::PrettyStackTraceString CrashInfo("Building compiler instance");

    // Create diagnostics

    IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = BuildDiagnosticOptions();
    IntrusiveRefCntPtr<DiagnosticsEngine> Diags = CreateDiagnostics(DiagOpts);

    // Create all options.

    FileSystemOptions fileSystemOpts;
    std::shared_ptr<TargetOptions> TargetOpts = std::make_shared<TargetOptions>();
    FrontendOptions *FrontendOpts = new FrontendOptions();
    CodeGenOptions *CodeGenOpts = new CodeGenOptions();
    BuildOptions(fileSystemOpts, TargetOpts, &*FrontendOpts, &*CodeGenOpts);
    
    if (doExecute) {
        CI = std::make_shared<CompilerInstance>(Diags,
                                                std::move(fileSystemOpts),
                                                FrontendOpts,
                                                CodeGenOpts,
                                                std::move(TargetOpts));
        if (!CI) {
            llvm::errs() << "Error while creating compiler instance!" << "\n";
            exit(1);
        }
    }

    return *CI;
}

// Diagnostics
IntrusiveRefCntPtr<DiagnosticsEngine> Driver::CreateDiagnostics(IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts) {
    FLY_DEBUG("Driver", "CreateDiagnostics");
    DiagOpts = new DiagnosticOptions;
    TextDiagnosticPrinter *DiagClient = new TextDiagnosticPrinter(llvm::errs(), &*DiagOpts);
    StringRef ExeBasename(llvm::sys::path::stem(Path));
    DiagClient->setPrefix(std::string(ExeBasename));
    IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
    IntrusiveRefCntPtr<DiagnosticsEngine> Diags = new DiagnosticsEngine(DiagID, &*DiagOpts, DiagClient);

    llvm::raw_ostream *OS = &llvm::errs();
    std::error_code EC;
    std::unique_ptr<llvm::raw_ostream> StreamOwner;

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

        // Chain in the diagnostic client which will log the diagnostics.
        auto Logger = std::make_unique<LogDiagnosticPrinter>(*OS, DiagOpts.get(), std::move(StreamOwner));

        if (Diags->ownsClient()) {
            Diags->setClient(
                    new ChainedDiagnosticConsumer(Diags->takeClient(), std::move(Logger)));
        } else {
            Diags->setClient(
                    new ChainedDiagnosticConsumer(Diags->getClient(), std::move(Logger)));
        }
    }

    ProcessWarningOptions(*Diags, *DiagOpts, /*ReportDiags=*/false);

    return std::move(Diags);
}

IntrusiveRefCntPtr<DiagnosticOptions> Driver::BuildDiagnosticOptions() {
    FLY_DEBUG("Driver", "BuildDiagnosticOptions");
    IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts(new DiagnosticOptions);
    DiagOpts->DiagnosticLogFile = ArgList.hasArg(options::OPT_LOG_FILE) ?
            std::string(ArgList.getLastArgValue(options::OPT_LOG_FILE)) : "";
    DiagOpts->IgnoreWarnings = ArgList.hasArg(options::OPT_NO_WARNING);
    return std::move(DiagOpts);
}

void Driver::BuildOptions(FileSystemOptions &FileSystemOpts,
                            std::shared_ptr<TargetOptions> &TargetOpts,
                            FrontendOptions *FrontendOpts,
                            CodeGenOptions *CodeGenOpts) {
    FLY_DEBUG_MESSAGE("Driver", "BuildOptions", "Parsing command line arguments");
    llvm::PrettyStackTraceString CrashInfo("Command line argument parsing");

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
    } else if (ArgList.hasArg(options::OPT_VERSION_SHORT)) {
        printVersion(false);
        doExecute = false;
        return;
    }

    // Show Help
    else if (ArgList.hasArg(options::OPT_HELP)) {
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
        for (const llvm::opt::Arg *A : ArgList.filtered(options::OPT_INPUT)) {
            FLY_DEBUG_MESSAGE("Driver", "BuildOptions",
                              "Set OPT_INPUT=" << A->getValue());
            FrontendOpts->addInputFile(A->getValue());
        }
        if (FrontendOpts->getInputFiles().empty()) {
            llvm::errs() << "no input files" << "\n";
            doExecute = false;
            return;
        }
    }

    // Enable Verbose Log
    if (ArgList.hasArg(options::OPT_VERBOSE)) {
        FLY_DEBUG_MESSAGE("Driver", "BuildOptions", "Set OPT_VERBOSE");
        FrontendOpts->Verbose = true;
    }

    // Configure Options

    // Set Output file
    if (ArgList.hasArg(options::OPT_OUTPUT)) {
        if (ArgList.hasArg(options::OPT_EMIT_LL) ||
                ArgList.hasArg(options::OPT_EMIT_BC) ||
                ArgList.hasArg(options::OPT_EMIT_AS) ||
                ArgList.hasArg(options::OPT_EMIT_NOTHING)) {
            llvm::errs() << "cannot specify -o when not emit object files\n";
            doExecute = false;
            return;
        }

        StringRef Out = ArgList.getLastArgValue(options::OPT_OUTPUT);
        FLY_DEBUG_MESSAGE("Driver", "BuildOptions", "Set OPT_OUTPUT=" << Out);
        FrontendOpts->setOutputFile(Out.str());
    } else if (ArgList.hasArg(options::OPT_OUTPUT_LIB)) {
        StringRef Out = ArgList.getLastArgValue(options::OPT_OUTPUT_LIB);
        FLY_DEBUG_MESSAGE("Driver", "BuildOptions", "Set OPT_OUTPUT_LIB=" << Out);
        FrontendOpts->setOutputFile(Out.str(), true);
    }

    // Set Working Directory
    if (const llvm::opt::Arg *A = ArgList.getLastArg(options::OPT_WORKING_DIR)) {
        FLY_DEBUG_MESSAGE("Driver", "BuildOptions",
                          "Set OPT_WORKING_DIR=" << A->getValue());
        FileSystemOpts.WorkingDir = A->getValue();
    }

    // Print Statistics
    if (ArgList.hasArg(options::OPT_PRINT_STATS)) {
        FLY_DEBUG_MESSAGE("Driver", "BuildOptions", "Set OPT_PRINT_STATS");
        FrontendOpts->ShowStats = true;
    }

    // Show Timers
    if (ArgList.hasArg(options::OPT_FTIME_REPORT)) {
        FLY_DEBUG_MESSAGE("Driver", "BuildOptions", "Set OPT_FTIME_REPORT");
        FrontendOpts->ShowTimers = true;
    }

    // Output Statistics file
    if (ArgList.hasArg(options::OPT_STATS_FILE)) {
        FrontendOpts->StatsFile = std::string(ArgList.getLastArgValue(options::OPT_STATS_FILE));
        FLY_DEBUG_MESSAGE("Driver", "BuildOptions",
                          "Set OPT_STATS_FILE=" << FrontendOpts->StatsFile);
    }

    // Emit different kind of file
    if (ArgList.hasArg(options::OPT_EMIT_LL)) {
        FLY_DEBUG_MESSAGE("Driver", "BuildOptions", "Set OPT_EMIT_LL");
        FrontendOpts->BackendAction = BackendActionKind::Backend_EmitLL;
        FrontendOpts->setOutputFile("");
    } else if (ArgList.hasArg(options::OPT_EMIT_BC)) {
        FLY_DEBUG_MESSAGE("Driver", "BuildOptions", "Set OPT_EMIT_BC");
        FrontendOpts->BackendAction = BackendActionKind::Backend_EmitBC;
        FrontendOpts->setOutputFile("");
    } else if (ArgList.hasArg(options::OPT_EMIT_AS)) {
        FLY_DEBUG_MESSAGE("Driver", "BuildOptions", "Set OPT_EMIT_AS");
        FrontendOpts->BackendAction =BackendActionKind::Backend_EmitAssembly;
        FrontendOpts->setOutputFile("");
    } else if (ArgList.hasArg(options::OPT_EMIT_NOTHING)) {
        FLY_DEBUG_MESSAGE("Driver", "BuildOptions", "Set OPT_EMIT_NOTHING");
        FrontendOpts->BackendAction = BackendActionKind::Backend_EmitNothing;
        FrontendOpts->setOutputFile("");
    } else {
        FrontendOpts->BackendAction = BackendActionKind::Backend_EmitObj;
    }

    // Lib produce Obj files and need Header files
    if (ArgList.hasArg(options::OPT_OUTPUT_LIB)) {
        FrontendOpts->LibraryGen = true;
        FrontendOpts->BackendAction = BackendActionKind::Backend_EmitObj;
        FrontendOpts->HeaderGen = true;
    }

    // Header Generator
    if (ArgList.hasArg(options::OPT_HEADER_GENERATOR)) {
        FrontendOpts->HeaderGen = true;
    }

    // Target Options

    // Target Triple
    if (const llvm::opt::Arg *A = ArgList.getLastArg(options::OPT_TARGET)) {
        TargetOpts->Triple = A->getValue();
        FLY_DEBUG_MESSAGE("Driver", "BuildOptions",
                          "Set OPT_TARGET=" << TargetOpts->Triple);
    } else {
        TargetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
    }

    // Target Machine Code Model
    if (ArgList.hasArg(options::OPT_MC_MODEL)) {
        TargetOpts->CodeModel = std::string(ArgList.getLastArgValue(options::OPT_MC_MODEL));
        FLY_DEBUG_MESSAGE("Driver", "BuildOptions",
                          "Set OPT_MC_MODEL=" << TargetOpts->CodeModel);
    } else {
        TargetOpts->CodeModel = "default";
    }

    // Target CPU
    if (ArgList.hasArg(options::OPT_TARGET_CPU)) {
        TargetOpts->CPU = std::string(ArgList.getLastArgValue(options::OPT_TARGET_CPU));
        FLY_DEBUG_MESSAGE("Driver", "BuildOptions",
                          "Set OPT_TARGET_CPU=" << TargetOpts->CPU);
    }

    // CodeGen Options

    // Code Model
    CodeGenOpts->CodeModel = TargetOpts->CodeModel;

    // Thread Model
    if (ArgList.hasArg(options::OPT_MTHREAD_MODEL)) {
        CodeGenOpts->ThreadModel = std::string(ArgList.getLastArgValue(options::OPT_MTHREAD_MODEL));
        FLY_DEBUG_MESSAGE("Driver", "BuildOptions",
                          "Set OPT_MTHREAD_MODEL=" << CodeGenOpts->ThreadModel);
    } else {
        CodeGenOpts->ThreadModel = "posix";
    }
    if (CodeGenOpts->ThreadModel != "posix" && CodeGenOpts->ThreadModel != "single")
        llvm::errs() << ArgList.getLastArg(options::OPT_MTHREAD_MODEL)->getAsString(ArgList)
                << CodeGenOpts->ThreadModel;
}

bool Driver::Execute() {
    FLY_DEBUG("Driver", "Execute");
    bool Success = true;

    if (doExecute) {
        Frontend Front(*CI);
        Success = Front.Execute();

        if (!CI->getFrontendOptions().getOutputFile().getFile().empty()) {
            const llvm::Triple &T = TargetInfo::CreateTargetInfo(CI->getDiagnostics(),
                                                                 CI->getTargetOptions())->getTriple();
            ToolChain *TC = new ToolChain(CI->getDiagnostics(), T);
            Success = TC->BuildOutput(Front.getOutputFiles(), CI->getFrontendOptions());

            // Delete Output Files on Library generation
            if (CI->getFrontendOptions().LibraryGen) {
                for (auto &Output: Front.getOutputFiles()) {
                    FLY_DEBUG_MESSAGE("Driver", "Execute",
                                      "Delete Output File " << Output);
                    llvm::sys::fs::remove(Output, false);
                }
            }
        }
    }
    FLY_DEBUG_MESSAGE("Driver", "Execute", "return " << Success);

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
