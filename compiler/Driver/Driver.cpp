//===--------------------------------------------------------------------------------------------------------------===//
// src/Driver/Driver.cpp - Driver
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Driver/Driver.h"
#include "Driver/ToolChain.h"
#include "Config/Config.h"
#include "Basic/PrettyStackTrace.h"
#include "Basic/FileSystemOptions.h"
#include "Frontend/Frontend.h"
#include "Frontend/ChainedDiagnosticConsumer.h"
#include "Frontend/LogDiagnosticPrinter.h"
#include "CodeGen/BackendUtil.h"
#include "Basic/Debug.h"
#include <CLI/CLI.hpp>
#include <memory>
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/WithColor.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/Support/CrashRecoveryContext.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Program.h"

#include <utility>

using namespace fly;

// Shows single-dash aliases inline with the canonical --name in help output.
class FlyFormatter : public CLI::Formatter {
public:
    std::string make_option_name(const CLI::Option *opt, bool is_positional) const override {
        std::string name = CLI::Formatter::make_option_name(opt, is_positional);
        if (name == "--help")    return "-help, --help";
        if (name == "--version") return "-version, --version";
        return name;
    }
};

std::string GetExecutablePath(const char *Argv0) {
    SmallString<128> ExecutablePath(Argv0);
    if (!llvm::sys::fs::exists(ExecutablePath))
        if (llvm::ErrorOr<std::string> P = llvm::sys::findProgramByName(ExecutablePath))
            ExecutablePath = *P;
    return std::string(ExecutablePath.str());
}

static llvm::ArrayRef<const char *> initDriverArgs() {
    static const char *Argv[] = {"fly"};
    return {Argv, 1};
}

Driver::Driver() : Driver(initDriverArgs()) {}

// CLI11 requires "--" for multi-char long options; Fly (like clang) accepts
// single-dash long options (-debug, -help …).
// Build a normalised argv array (including argv[0]) and use CLI11's
// argc/argv overload, which has correct left-to-right positional semantics.
static std::vector<std::string>
normaliseDashes(llvm::ArrayRef<const char *> ArrArgs) {
    std::vector<std::string> out;
    out.reserve(ArrArgs.size());
    for (const char *raw : ArrArgs) {
        std::string s(raw);
        // -debug → --debug  (single-dash, multi-char, not -o<joined-value>)
        if (s.size() > 2 && s[0] == '-' && s[1] != '-' && s[1] != 'o')
            s = "-" + s;
        out.push_back(std::move(s));
    }
    return out;
}

Driver::Driver(llvm::ArrayRef<const char *> ArrArgs) :
        Path(GetExecutablePath(ArrArgs[0])) {
    Name = std::string(llvm::sys::path::filename(Path));
    Dir  = std::string(llvm::sys::path::parent_path(Path));
    InstalledDir = Dir;

    // Normalise all args (including argv[0]) then use CLI11's argc/argv overload,
    // which correctly handles left-to-right positional/option interleaving.
    std::vector<std::string> NormStrs = normaliseDashes(ArrArgs);
    std::vector<const char *> NormArgv;
    NormArgv.reserve(NormStrs.size());
    for (const auto &s : NormStrs)
        NormArgv.push_back(s.c_str());

    CLI::App app("Fly Compiler", "fly");
    app.formatter(std::make_shared<FlyFormatter>());
    // Disable built-in --help / --version so we control the flags ourselves.
    app.set_help_flag("");
    app.set_version_flag("");

    bool showHelp    = false;
    bool showVersion = false;
    bool debugFlag   = false;

    // Multi-char options use "--" prefix (CLI11 requirement).
    // Single-dash input (-help, -debug, …) is normalised to "--" by normaliseDashes().
    app.add_flag("--help",    showHelp,    "Display available options");
    app.add_flag("--version", showVersion, "Print version information");
    app.add_flag("--debug",         debugFlag,   "Print debug messages");
    app.add_flag("-v,--verbose",    Verbose,     "Show commands to run and use verbose output");
    app.add_flag("-w,--no-warning", NoWarnings,  "Suppress all warnings");
    app.add_flag("--emit-ll",       EmitLL,      "Produce LLVM IR output");
    app.add_flag("--emit-bc",       EmitBC,      "Produce bitcode output");
    app.add_flag("--emit-as",       EmitAS,      "Produce assembly output");
    app.add_flag("--no-output",     NoOutput,    "Produce no output");
    app.add_flag("--header",        HeaderGen,   "Generate header file");
    app.add_flag("--print-stats",   PrintStats,  "Print performance metrics and statistics");
    app.add_flag("--ftime-report",  FtimeReport, "Show timers for individual actions");

    app.add_option("-o",            OutputFile,   "Write output to <file>");
    app.add_flag("--lib",           OutputLib,    "Produce a library archive (.a/.lib)");
    app.add_option("--log-file",    LogFile,      "Log diagnostics to <file>");
    app.add_option("--log-format",  LogFormat,    "Log format: txt (default) or json")->check(CLI::IsMember({"txt", "json"}));
    app.add_option("--mcmodel",     McModel,      "Set memory code model");
    app.add_option("--mthread-model", MthreadModel, "Set memory thread model");
    app.add_option("--target",      Target,       "Generate code for the given target");
    app.add_option("--target-cpu",  TargetCpu,    "Generate code for the given CPU");
    app.add_option("--stats-file",  StatsFile,    "Filename to write statistics to");
    app.add_option("--working-dir", WorkingDir,   "Resolve file paths relative to the specified directory");

    // Remaining non-option args become input files.
    app.allow_extras(true);

    try {
        app.parse((int)NormArgv.size(), NormArgv.data());
    } catch (const CLI::ParseError &e) {
        if (e.get_exit_code() != 0) {
            llvm::errs() << "error: " << e.what() << "\n";
            llvm::errs() << "Use '" << Path << " --help' for a complete list of options.\n";
            doExecute = false;
        }
        return;
    }

    // Collect positional (input) files from unmatched args.
    // Unknown --flags in remaining are reported as errors.
    for (const auto &r : app.remaining()) {
        if (!r.empty() && r[0] == '-') {
            llvm::errs() << "error: unknown option: " << r << "\n";
            llvm::errs() << "Use '" << Path << " --help' for a complete list of options.\n";
            doExecute = false;
            return;
        }
        InputFiles.push_back(r);
    }

    if (debugFlag) {
        DebugEnabled = true;
        FLY_DEBUG_START_MSG("Driver", "Driver", "Set -debug");
    }

    if (showVersion) {
        printVersion();
        doExecute = false;
        return;
    }

    if (showHelp) {
        llvm::outs() << app.help();
        doExecute = false;
        return;
    }
}

Driver::~Driver() {
    DebugEnabled = false;
}

CompilerInstance &Driver::BuildCompilerInstance() {
    FLY_DEBUG_START("Driver", "BuildCompilerInstance");
    llvm::PrettyStackTraceString CrashInfo("Building compiler instance");

    IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = BuildDiagnosticOptions();
    IntrusiveRefCntPtr<DiagnosticsEngine> Diags    = CreateDiagnostics(DiagOpts);

    FileSystemOptions FileSystemOpts;
    std::shared_ptr<TargetOptions> TargetOpts = std::make_shared<TargetOptions>();
    FrontendOptions *FrontendOpts = new FrontendOptions();
    CodeGenOptions  *CodeGenOpts  = new CodeGenOptions();
    BuildOptions(FileSystemOpts, TargetOpts, FrontendOpts, CodeGenOpts);

    CI = std::make_shared<CompilerInstance>(Diags,
                                            std::move(FileSystemOpts),
                                            std::move(TargetOpts),
                                            FrontendOpts,
                                            CodeGenOpts);
    if (!CI) {
        llvm::errs() << "Error while creating compiler instance!\n";
        exit(1);
    }

    return *CI;
}

IntrusiveRefCntPtr<DiagnosticsEngine>
Driver::CreateDiagnostics(IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts) {
    FLY_DEBUG_START("Driver", "CreateDiagnostics");
    TextDiagnosticPrinter *DiagClient = new TextDiagnosticPrinter(llvm::errs(), &*DiagOpts);
    StringRef ExeBasename(llvm::sys::path::stem(Path));
    DiagClient->setPrefix(std::string(ExeBasename));
    IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
    IntrusiveRefCntPtr<DiagnosticsEngine> Diags =
        new DiagnosticsEngine(DiagID, &*DiagOpts, DiagClient);

    llvm::raw_ostream *OS = &llvm::errs();
    std::error_code EC;
    std::unique_ptr<llvm::raw_ostream> StreamOwner;

    if (!DiagOpts->DiagnosticLogFile.empty()) {
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
        auto Logger = std::make_unique<LogDiagnosticPrinter>(
            *OS, DiagOpts.get(), std::move(StreamOwner));
        if (!OutputFile.empty())
            Logger->setMainFilename(OutputFile);
        if (LogFormat == "json")
            Logger->setLogFormat(LogDiagnosticPrinter::LogFormat::Json);
        {
            LogDiagnosticPrinter::InvocationInfo Info;
            Info.InputFiles   = InputFiles;
            Info.Target       = Target;
            Info.TargetCpu    = TargetCpu;
            Info.McModel      = McModel;
            Info.MthreadModel = MthreadModel;
            Info.WorkingDir   = WorkingDir;
            Info.OutputLib    = OutputLib;
            Info.Verbose      = Verbose;
            Info.NoWarnings   = NoWarnings;
            Info.EmitLL       = EmitLL;
            Info.EmitBC       = EmitBC;
            Info.EmitAS       = EmitAS;
            Info.NoOutput     = NoOutput;
            Info.HeaderGen    = HeaderGen;
            Info.PrintStats   = PrintStats;
            Info.FtimeReport  = FtimeReport;
            Logger->setInvocation(Info);
        }
        if (Diags->ownsClient())
            Diags->setClient(new ChainedDiagnosticConsumer(Diags->takeClient(), std::move(Logger)));
        else
            Diags->setClient(new ChainedDiagnosticConsumer(Diags->getClient(), std::move(Logger)));
    }

    ProcessWarningOptions(*Diags, *DiagOpts, /*ReportDiags=*/false);
    return std::move(Diags);
}

IntrusiveRefCntPtr<DiagnosticOptions> Driver::BuildDiagnosticOptions() {
    FLY_DEBUG_START("Driver", "BuildDiagnosticOptions");
    IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts(new DiagnosticOptions);
    DiagOpts->DiagnosticLogFile = LogFile;
    DiagOpts->IgnoreWarnings    = NoWarnings;
    return std::move(DiagOpts);
}

void Driver::BuildOptions(FileSystemOptions &FileSystemOpts,
                          std::shared_ptr<TargetOptions> &TargetOpts,
                          FrontendOptions *FrontendOpts,
                          CodeGenOptions  *CodeGenOpts) {
    FLY_DEBUG_START_MSG("Driver", "BuildOptions", "Parsing command line arguments");
    llvm::PrettyStackTraceString CrashInfo("Command line argument parsing");

    if (!doExecute) return;

    // Input files
    if (InputFiles.empty()) {
        llvm::errs() << "no input files\n";
        doExecute = false;
        return;
    }
    for (const auto &F : InputFiles) {
        FLY_DEBUG_START_MSG("Driver", "BuildOptions", "Set input=" << F);
        FrontendOpts->addInputFile(F.c_str());
    }

    // Verbose
    if (Verbose) {
        FLY_DEBUG_START_MSG("Driver", "BuildOptions", "Set -verbose");
        FrontendOpts->Verbose = true;
    }

    // Output file
    if (!OutputFile.empty()) {
        if (EmitLL || EmitBC || EmitAS || NoOutput) {
            llvm::errs() << "cannot specify -o when not emit object files\n";
            doExecute = false;
            return;
        }
        FLY_DEBUG_START_MSG("Driver", "BuildOptions", "Set -o=" << OutputFile);
        FrontendOpts->setOutputFile(OutputFile);
    }

    // Working directory
    if (!WorkingDir.empty()) {
        FLY_DEBUG_START_MSG("Driver", "BuildOptions", "Set -working-dir=" << WorkingDir);
        FileSystemOpts.WorkingDir = WorkingDir;
    }

    // Statistics
    if (PrintStats) {
        FLY_DEBUG_START_MSG("Driver", "BuildOptions", "Set -print-stats");
        FrontendOpts->ShowStats = true;
    }
    if (!StatsFile.empty()) {
        FrontendOpts->StatsFile = StatsFile;
        FLY_DEBUG_START_MSG("Driver", "BuildOptions", "Set -stats-file=" << StatsFile);
    }

    // Timers
    if (FtimeReport) {
        FLY_DEBUG_START_MSG("Driver", "BuildOptions", "Set -ftime-report");
        FrontendOpts->ShowTimers = true;
    }

    // Backend emit action
    if (EmitLL) {
        FLY_DEBUG_START_MSG("Driver", "BuildOptions", "Set -emit-ll");
        FrontendOpts->BackendAction = BackendActionKind::Backend_EmitLL;
        FrontendOpts->setOutputFile("");
    } else if (EmitBC) {
        FLY_DEBUG_START_MSG("Driver", "BuildOptions", "Set -emit-bc");
        FrontendOpts->BackendAction = BackendActionKind::Backend_EmitBC;
        FrontendOpts->setOutputFile("");
    } else if (EmitAS) {
        FLY_DEBUG_START_MSG("Driver", "BuildOptions", "Set -emit-as");
        FrontendOpts->BackendAction = BackendActionKind::Backend_EmitAssembly;
        FrontendOpts->setOutputFile("");
    } else if (NoOutput) {
        FLY_DEBUG_START_MSG("Driver", "BuildOptions", "Set -no-output");
        FrontendOpts->BackendAction = BackendActionKind::Backend_EmitNothing;
        FrontendOpts->setOutputFile("");
    } else {
        FrontendOpts->BackendAction = BackendActionKind::Backend_EmitObj;
    }

    // Library
    if (OutputLib) {
        FrontendOpts->CreateLibrary = true;
        FrontendOpts->BackendAction = BackendActionKind::Backend_EmitObj;
        FrontendOpts->CreateHeader  = true;
    }

    // Header generator
    if (HeaderGen)
        FrontendOpts->CreateHeader = true;

    // Target triple
    if (!Target.empty()) {
        TargetOpts->Triple = Target;
        FLY_DEBUG_START_MSG("Driver", "BuildOptions", "Set --target=" << Target);
    } else {
        TargetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
    }

    // Code model
    if (!McModel.empty()) {
        TargetOpts->CodeModel = McModel;
        FLY_DEBUG_START_MSG("Driver", "BuildOptions", "Set -mcmodel=" << McModel);
    } else {
        TargetOpts->CodeModel = "default";
    }

    // CPU
    if (!TargetCpu.empty()) {
        TargetOpts->CPU = TargetCpu;
        FLY_DEBUG_START_MSG("Driver", "BuildOptions", "Set --target-cpu=" << TargetCpu);
    }

    // CodeGen options
    CodeGenOpts->CodeModel = TargetOpts->CodeModel;

    if (!MthreadModel.empty()) {
        CodeGenOpts->ThreadModel = MthreadModel;
        FLY_DEBUG_START_MSG("Driver", "BuildOptions", "Set -mthread-model=" << MthreadModel);
    } else {
        CodeGenOpts->ThreadModel = "posix";
    }
    if (CodeGenOpts->ThreadModel != "posix" && CodeGenOpts->ThreadModel != "single")
        llvm::errs() << "invalid thread model: " << CodeGenOpts->ThreadModel << "\n";
}

void Driver::printVersion(bool full) {
    if (full)
        llvm::outs() << "Fly version " << FLY_VERSION << " (https://flylang.org)\n";
    else
        llvm::outs() << FLY_VERSION << "\n";
}

bool Driver::Execute() {
    FLY_DEBUG_START("Driver", "Execute");
    bool Success = true;

    if (doExecute) {
        Frontend Front(*CI);
        Success = Front.Execute();

        if (!CI->getFrontendOptions().getOutputFile().empty()) {
            std::unique_ptr<TargetInfo> TI(TargetInfo::CreateTargetInfo(
                CI->getDiagnostics(), CI->getTargetOptions()));
            const llvm::Triple &T = TI->getTriple();
            std::unique_ptr<ToolChain> TC = std::make_unique<ToolChain>(
                CI->getDiagnostics(), T, CI->getCodeGenOptions());
            Success = TC->BuildOutput(Front.getOutputFiles(), CI->getFrontendOptions());

            if (CI->getFrontendOptions().CreateLibrary) {
                for (auto &Output : Front.getOutputFiles()) {
                    if (llvm::StringRef(Output).ends_with(".fly.h"))
                        continue;
                    FLY_DEBUG_START_MSG("Driver", "Execute", "Delete Output File " << Output);
                    const std::error_code &EC = llvm::sys::fs::remove(Output, false);
                    if (EC) {
                        CI->getDiagnostics().Report(diag::err_drv_archive) << EC.message();
                        return false;
                    }
                }
            }
        }
    }
    FLY_DEBUG_END("Driver", "Execute");
    return Success;
}
