//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Driver/Driver.cpp - compiler driver
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

#include <utility>
#include <filesystem>

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
    // Resolve the *real* running executable, independent of argv[0]/$PATH — the
    // same mechanism Clang's driver uses and the C++/LLVM equivalent of Rust's
    // std::env::current_exe / rustc's current_dll_path. MainAddr is the address
    // of a local symbol; llvm::sys::fs::getMainExecutable uses it together with
    // /proc/self/exe (Linux), dladdr (macOS) and GetModuleFileName (Windows),
    // falling back to an argv0+$PATH search only if those fail.
    //
    // This avoids the bare-name trap: invoking `fly` (found via $PATH) from a
    // directory that happens to contain a `fly/` subdirectory previously made
    // llvm::sys::fs::exists("fly") return true, leaving the path unresolved and
    // breaking <bin>/../lib stdlib discovery.
    void *MainAddr = (void *)(intptr_t)GetExecutablePath;
    return llvm::sys::fs::getMainExecutable(Argv0, MainAddr);
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
        // -debug → --debug  (single-dash, multi-char)
        // Exclude: -o<file>, -O<level>, -j<n>, -L<dir>, -I<dir>
        if (s.size() > 2 && s[0] == '-' && s[1] != '-'
                && s[1] != 'o' && s[1] != 'O' && s[1] != 'j'
                && s[1] != 'L' && s[1] != 'I')
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
    app.add_flag("--debug-symbols", DebugSymbols,"Emit DWARF debug information (no verbose logging)");
    app.add_flag("-v,--verbose",    Verbose,     "Show commands to run and use verbose output");
    app.add_flag("-w,--no-warning", NoWarnings,  "Suppress all warnings");
    // Output format: WHAT the backend produces. Default is an object file.
    app.add_flag("--emit-ll",       EmitLL,      "Produce LLVM IR output (.ll)");
    app.add_flag("--emit-bc",       EmitBC,      "Produce bitcode output (.bc)");
    app.add_flag("--emit-as",       EmitAS,      "Produce assembly output (.s)");
    // Compilation stage: HOW FAR to go. Default is to link.
    app.add_flag("--no-output",     NoOutput,    "Parse and analyse only, produce no output");
    app.add_flag("-c",              CompileOnly, "Compile only, do not link");
    app.add_flag("--header",        HeaderGen,   "Generate header file");
    app.add_flag("--print-stats",   PrintStats,  "Print performance metrics and statistics");
    app.add_flag("--ftime-report",  FtimeReport, "Show timers for individual actions");

    app.add_option("-o",            OutputFile,   "Write output to <file>");
    // Linked-artifact shape: what the LINK step produces. Independent of the format
    // above — combining these with --emit-* is a diagnosed error, never an override.
    app.add_flag("--lib,--lib-static",      OutputLib,    "Produce a static library (.a/.lib)");
    app.add_flag("--lib-dyn,--lib-dynamic", OutputShared, "Produce a dynamic library (.so/.dylib/.dll)");
    app.add_option("--log-file",    LogFile,      "Log diagnostics to <file>");
    app.add_option("--log-format",  LogFormat,    "Log format: txt (default) or json")->check(CLI::IsMember({"txt", "json"}));
    app.add_option("--mcmodel",     McModel,      "Set memory code model");
    app.add_option("--mthread-model", MthreadModel, "Set memory thread model");
    app.add_option("--jobs,-j",     Jobs,         "Number of threads for LLVM internal parallelism (0 = auto)");
    app.add_option("-O",            OptLevel,     "Optimisation level: 0 (none), 1, 2, 3 (max). Default: 0 for debug, 3 for release.");
    app.add_option("--target",      Target,       "Generate code for the given target");
    app.add_option("--target-cpu",  TargetCpu,    "Generate code for the given CPU");
    app.add_option("--stats-file",  StatsFile,    "Filename to write statistics to");
    app.add_option("--working-dir", WorkingDir,   "Resolve file paths relative to the specified directory");
    app.add_option("-L",            LibDirs,      "Add <dir> to the library search path for namespace resolution")->allow_extra_args(false);
    app.add_option("--src-dir",     SrcDirs,      "Source root directory: inputs and import-based dependencies are discovered here (default: current directory)")->allow_extra_args(false);
    app.add_option("--out-dir",     OutDirOpt,    "Directory for all generated build outputs (created if missing)");
    app.add_option("--link-lib",   LinkLibs,     "Link against external C library NAME (passed as -lNAME to the linker)")->allow_extra_args(false);
    app.add_option("--llvm-lib-dir", LlvmLibDir, "Directory holding the LLVM libraries (libLLVM-20.so / LLVM-20.lib) for programs using the LLVM C-API (default: probed next to the fly binary)");

    // No positional arguments exist: fly compiles a source DIRECTORY (--src-dir,
    // default: current directory), never named files. Extras are kept only to be
    // diagnosed with a dedicated message below.
    app.allow_extras(true);

    try {
        app.parse((int)NormArgv.size(), NormArgv.data());
    } catch (const CLI::ParseError &e) {
        if (e.get_exit_code() != 0) {
            llvm::errs() << "error: " << e.what() << "\n";
            llvm::errs() << "Use '" << Path << " --help' for a complete list of options.\n";
            doExecute = false;
            HadOptionError = true;
        }
        return;
    }

    // Unknown --flags are reported as such; any other positional is rejected —
    // sources are not named on the command line, they are discovered from the
    // source directory.
    for (const auto &r : app.remaining()) {
        if (!r.empty() && r[0] == '-') {
            llvm::errs() << "error: unknown option: " << r << "\n";
            llvm::errs() << "Use '" << Path << " --help' for a complete list of options.\n";
        } else {
            llvm::errs() << "error: unexpected argument '" << r
                         << "': fly compiles a source directory, use --src-dir <dir>"
                            " (default: current directory)\n";
        }
        doExecute = false;
        HadOptionError = true;
        return;
    }

    if (debugFlag) {
        DebugLog = true;
        DebugSymbols = true;
        FLY_DEBUG_MSG("Set -debug");
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
    DebugLog = false;
}

CompilerInstance &Driver::BuildCompilerInstance() {
    FLY_DEBUG_SCOPE("Driver", "BuildCompilerInstance");
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
    FLY_DEBUG_SCOPE("Driver", "CreateDiagnostics");
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
    FLY_DEBUG_SCOPE("Driver", "BuildDiagnosticOptions");
    IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts(new DiagnosticOptions);
    DiagOpts->DiagnosticLogFile = LogFile;
    DiagOpts->IgnoreWarnings    = NoWarnings;
    return std::move(DiagOpts);
}

void Driver::BuildOptions(FileSystemOptions &FileSystemOpts,
                          std::shared_ptr<TargetOptions> &TargetOpts,
                          FrontendOptions *FrontendOpts,
                          CodeGenOptions  *CodeGenOpts) {
    FLY_DEBUG_SCOPE_MSG("Driver", "BuildOptions", "Parsing command line arguments");
    llvm::PrettyStackTraceString CrashInfo("Command line argument parsing");

    if (!doExecute) return;

    // Directory mode: the Frontend discovers the input files from the source root
    // (--src-dir, default: current directory) — main() for executables, suite
    // declarations in test mode, every source for --lib/--lib-dyn.
    FrontendOpts->DiscoverInputs = true;

    // Library search dirs (-L)
    for (const auto &D : LibDirs) {
        FLY_DEBUG_MSG("Set -L=" << D);
        FrontendOpts->LibDirs.push_back(D);
    }

    // Source root (--src-dir) for import-based dependency discovery. The project
    // root defaults to the current directory (resolution always runs); --src-dir
    // overrides that root and is therefore meaningful at most ONCE.
    if (SrcDirs.size() > 1) {
        llvm::errs() << "error: --src-dir may be specified only once\n";
        doExecute = false;
        HadOptionError = true;
        return;
    }
    for (const auto &D : SrcDirs) {
        FLY_DEBUG_MSG("Set --src-dir=" << D);
        FrontendOpts->SrcDirs.push_back(D);
    }

    // Output directory (--out-dir): all generated build outputs go here.
    if (!OutDirOpt.empty()) {
        FLY_DEBUG_MSG("Set --out-dir=" << OutDirOpt);
        FrontendOpts->OutDir = OutDirOpt;
    }

    // Auto-discover stdlib relative to the fly binary (<bin_dir>/../lib).
    // Works in the build tree (build/bin → build/lib, copied by CMake at build time)
    // and when installed (/usr/local/bin → /usr/local/lib).
    // Stored in StdLibDir and loaded as .fly source (not .fly.h headers).
    {
        namespace fs = std::filesystem;
        std::error_code ec;
        auto candidate = fs::canonical(
            fs::path(Dir) / ".." / "lib", ec);
        if (!ec && fs::is_directory(candidate)) {
            FrontendOpts->StdLibDir = candidate.string();
            CodeGenOpts->RuntimeLibDir = candidate.string();
        }
    }

    // LLVM lib dir → linker search path (-L / /libpath:), so programs that use
    // the LLVM C-API (the compiler's own CodeGen/Target suites reference
    // libLLVM-20.so → -lLLVM-20) link against the fork LLVM without a system
    // install. Probe order mirrors the self-host ToolChain.getLLVMLibDir():
    // the --llvm-lib-dir override, then <exe_dir>/llvm/lib (release bundle),
    // then <exe_dir>/../llvm/lib (this repo's build tree: build/bin →
    // build/llvm/lib), then <exe_dir>/../../llvm/lib (staged bootstrap:
    // build/stage0/bin → build/llvm/lib). Left empty when none exists —
    // ordinary user programs don't reference LLVM.
    {
        namespace fs = std::filesystem;
        std::error_code ec;
        if (!LlvmLibDir.empty() && fs::is_directory(LlvmLibDir, ec)) {
            CodeGenOpts->ToolchainLibDir = LlvmLibDir;
        } else {
            for (const fs::path &Candidate : {fs::path(Dir) / "llvm" / "lib",
                                              fs::path(Dir) / ".." / "llvm" / "lib",
                                              fs::path(Dir) / ".." / ".." / "llvm" / "lib"}) {
                auto canon = fs::canonical(Candidate, ec);
                if (!ec && fs::is_directory(canon, ec)) {
                    CodeGenOpts->ToolchainLibDir = canon.string();
                    FLY_DEBUG_MSG("Set ToolchainLibDir=" << CodeGenOpts->ToolchainLibDir);
                    break;
                }
            }
        }
    }

    // External C libraries to link (--link-lib NAME → -lNAME in linker flags)
    for (const auto &Lib : LinkLibs) {
        std::string Flag = "-l" + Lib;
        if (std::find(CodeGenOpts->LinkerOptions.begin(), CodeGenOpts->LinkerOptions.end(), Flag)
                == CodeGenOpts->LinkerOptions.end())
            CodeGenOpts->LinkerOptions.push_back(Flag);
    }

    // Verbose
    if (Verbose) {
        FLY_DEBUG_MSG("Set -verbose");
        FrontendOpts->Verbose = true;
    }

    // Output file. -o is valid for the emit actions too: it names the single emitted
    // artifact (.ll/.bc/.s) and makes the Frontend lower every input into ONE module so
    // cross-file references resolve. Only --no-output, which emits nothing, conflicts.
    if (!OutputFile.empty()) {
        if (NoOutput) {
            llvm::errs() << "cannot specify -o with --no-output\n";
            doExecute = false;
            HadOptionError = true;
            return;
        }
        FLY_DEBUG_MSG("Set -o=" << OutputFile);
        FrontendOpts->setOutputFile(OutputFile);
    }

    // Working directory
    if (!WorkingDir.empty()) {
        FLY_DEBUG_MSG("Set -working-dir=" << WorkingDir);
        FileSystemOpts.WorkingDir = WorkingDir;
    }

    // Statistics
    if (PrintStats) {
        FLY_DEBUG_MSG("Set -print-stats");
        FrontendOpts->ShowStats = true;
    }
    if (!StatsFile.empty()) {
        FrontendOpts->StatsFile = StatsFile;
        FLY_DEBUG_MSG("Set -stats-file=" << StatsFile);
    }

    // Timers
    if (FtimeReport) {
        FLY_DEBUG_MSG("Set -ftime-report");
        FrontendOpts->ShowTimers = true;
    }

    // ── Option model: three independent axes ──────────────────────────────────
    //   FORMAT  --emit-ll | --emit-bc | --emit-as | (default: object)
    //   STAGE   --no-output (emit nothing) | -c (emit, don't link) | (default: link)
    //   SHAPE   --lib/--lib-static | --lib-dyn/--lib-dynamic — what the LINK produces
    //
    // No axis silently rewrites another: an incompatible request is diagnosed here
    // rather than resolved by precedence. --lib used to overwrite the format, so
    // `fly --emit-ll --lib x.fly` handed back an object file to someone who asked
    // for IR, without a word.
    const bool WantsLibrary = OutputLib || OutputShared;
    const bool NonObjFormat = EmitLL || EmitBC || EmitAS;

    if (OutputLib && OutputShared) {
        llvm::errs() << "cannot specify both --lib and --lib-dyn\n";
        doExecute = false;
        HadOptionError = true;
        return;
    }
    if (WantsLibrary && NonObjFormat) {
        llvm::errs() << "cannot specify --lib/--lib-dyn with --emit-ll/--emit-bc/--emit-as\n";
        doExecute = false;
        HadOptionError = true;
        return;
    }
    if (WantsLibrary && (CompileOnly || NoOutput)) {
        llvm::errs() << "cannot specify --lib/--lib-dyn with -c or --no-output\n";
        doExecute = false;
        HadOptionError = true;
        return;
    }

    // FORMAT. Clearing the output file when -o is absent is what selects PER-FILE
    // emission (one artifact per input); with -o all inputs are lowered into ONE
    // module so cross-file references resolve. That is the module-count axis and is
    // deliberately independent of the link decision made below.
    const bool HasExplicitOutput = !OutputFile.empty();
    if (EmitLL) {
        FLY_DEBUG_MSG("Set -emit-ll");
        FrontendOpts->BackendAction = BackendActionKind::Backend_EmitLL;
        if (!HasExplicitOutput) FrontendOpts->setOutputFile("");
    } else if (EmitBC) {
        FLY_DEBUG_MSG("Set -emit-bc");
        FrontendOpts->BackendAction = BackendActionKind::Backend_EmitBC;
        if (!HasExplicitOutput) FrontendOpts->setOutputFile("");
    } else if (EmitAS) {
        FLY_DEBUG_MSG("Set -emit-as");
        FrontendOpts->BackendAction = BackendActionKind::Backend_EmitAssembly;
        if (!HasExplicitOutput) FrontendOpts->setOutputFile("");
    } else if (NoOutput) {
        FLY_DEBUG_MSG("Set -no-output");
        FrontendOpts->BackendAction = BackendActionKind::Backend_EmitNothing;
        FrontendOpts->setOutputFile("");
    } else {
        FrontendOpts->BackendAction = BackendActionKind::Backend_EmitObj;
    }

    // STAGE. Linking is an explicit decision recorded once, not something re-derived
    // downstream from "the output file happens to be non-empty". Only object code can
    // be linked, so any other format stops at the artifact (as -c does).
    FrontendOpts->LinkStep = !CompileOnly && !NoOutput &&
        FrontendOpts->BackendAction == BackendActionKind::Backend_EmitObj;
    if (CompileOnly)
        FLY_DEBUG_MSG("Set -c: compile only, no link");

    // SHAPE of the linked artifact. These no longer touch BackendAction — an
    // incompatible combination was rejected above.
    if (OutputLib) {
        FrontendOpts->CreateLibrary = true;
        FrontendOpts->CreateHeader  = true;
        FLY_DEBUG_MSG("Set --lib: producing static library");
    } else if (OutputShared) {
        FrontendOpts->CreateSharedLib       = true;
        CodeGenOpts->Shared                 = true;
        CodeGenOpts->RelocationModel        = llvm::Reloc::PIC_;
        FrontendOpts->CreateHeader          = true;
        FLY_DEBUG_MSG("Set --lib-dyn: producing dynamic library with PIC");
    }

    // Auto-detect / auto-name the output when no explicit -o was given on a linking
    // build. AutoDetectOutputType() infers the type from the entry AST (main → exe;
    // suite → test exe; otherwise lib) and auto-names the output —
    // from the entry file's stem, or from the stem discovery chose (suite name /
    // source-root name). --lib / --lib-dyn are still honoured there (they force a
    // library even with a main); only the auto-naming applies.
    //
    // The -o guard is deliberate: an invocation that already knows its output keeps
    // full control and is never reinterpreted. Requiring LinkStep covers the rest:
    // there is nothing to auto-name when the build stops at a .ll/.bc/.s, at -c, or
    // at --no-output.
    if (OutputFile.empty() && FrontendOpts->LinkStep) {
        FrontendOpts->AutoDetectOutput = true;
        FLY_DEBUG_MSG("Set AutoDetectOutput (no -o, object backend)");
    }

    // Header generator
    if (HeaderGen)
        FrontendOpts->CreateHeader = true;

    // Target triple
    if (!Target.empty()) {
        TargetOpts->Triple = Target;
        FLY_DEBUG_MSG("Set --target=" << Target);
    } else {
        TargetOpts->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
    }

    // Code model
    if (!McModel.empty()) {
        TargetOpts->CodeModel = McModel;
        FLY_DEBUG_MSG("Set -mcmodel=" << McModel);
    } else {
        TargetOpts->CodeModel = "default";
    }

    // CPU
    if (!TargetCpu.empty()) {
        TargetOpts->CPU = TargetCpu;
        FLY_DEBUG_MSG("Set --target-cpu=" << TargetCpu);
    }

    // Debug symbols
    if (DebugSymbols) {
        FLY_DEBUG_MSG("Set --debug: emitting debug symbols");
        CodeGenOpts->DebugSymbols = true;
    }

    // NOTE: there is no --test / --suite here any more. The CLI test RUNNER was
    // removed in 0.13.15: the 0.14.x bootstrap runs every suite with the compiler
    // each stage just produced (STAGE=N), so the seed never had to run them, and
    // the reference kept a user-facing runner nothing exercised. What remains is
    // the LANGUAGE: CodeGenModule emits a suite's implicit main() for every SUITE
    // class it sees, so the reference still BUILDS a suite binary. What went with
    // the runner is RUNNING it, selecting one suite by name, filtering a
    // test-method, and enabling `test {}` blocks inside a plain main().

    // CodeGen options
    CodeGenOpts->CodeModel = TargetOpts->CodeModel;

    if (!MthreadModel.empty()) {
        CodeGenOpts->ThreadModel = MthreadModel;
        FLY_DEBUG_MSG("Set -mthread-model=" << MthreadModel);
    } else {
        CodeGenOpts->ThreadModel = "posix";
    }
    if (CodeGenOpts->ThreadModel != "posix" && CodeGenOpts->ThreadModel != "single")
        llvm::errs() << "invalid thread model: " << CodeGenOpts->ThreadModel << "\n";

    CodeGenOpts->Jobs = Jobs;
    FLY_DEBUG_MSG("Set --jobs=" << Jobs);

    if (OptLevel >= 0) {
        CodeGenOpts->OptimizationLevel = OptLevel;
        FLY_DEBUG_MSG("Set -O" << OptLevel);
    }
}

void Driver::printVersion(bool full) {
    if (full)
        llvm::outs() << "Fly version " << FLY_VERSION << " (https://flylang.org)\n";
    else
        llvm::outs() << FLY_VERSION << "\n";
}

bool Driver::Execute() {
    FLY_DEBUG_SCOPE("Driver", "Execute");
    // A command-line error is a FAILURE (exit 1) even though nothing executes —
    // matching the self-host driver. --help/--version stay a successful no-op.
    if (HadOptionError)
        return false;

    bool Success = true;

    if (doExecute) {
        Frontend Front(*CI);
        Success = Front.Execute();

        // Link when the stage asked for it (see the option model in BuildOptions).
        // An empty output file still means there is nothing to name the linked
        // artifact — that is the historical "compile only" spelling, which -c now
        // expresses explicitly.
        const FrontendOptions &FO = CI->getFrontendOptions();
        if (Success && FO.LinkStep && !FO.getOutputFile().empty()) {
            std::unique_ptr<TargetInfo> TI(TargetInfo::CreateTargetInfo(
                CI->getDiagnostics(), CI->getTargetOptions()));
            const llvm::Triple &T = TI->getTriple();
            std::unique_ptr<ToolChain> TC = std::make_unique<ToolChain>(
                CI->getDiagnostics(), T, CI->getCodeGenOptions());
            Success = TC->BuildOutput(Front.getOutputFiles(), CI->getFrontendOptions());

            if (CI->getFrontendOptions().CreateLibrary ||
                CI->getFrontendOptions().CreateSharedLib) {
                for (auto &Output : Front.getOutputFiles()) {
                    if (llvm::StringRef(Output).ends_with(".fly.h"))
                        continue;
                    FLY_DEBUG_MSG("Delete Output File " << Output);
                    const std::error_code &EC = llvm::sys::fs::remove(Output, false);
                    if (EC) {
                        CI->getDiagnostics().Report(diag::err_drv_archive) << EC.message();
                        return false;
                    }
                }
            }
        }
    }
    return Success;
}
