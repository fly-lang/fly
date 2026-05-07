//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/BackendUtil.cpp - CodeGen Backend Util
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/BackendUtil.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/TargetParser/Triple.h"
#include "llvm/Analysis/StackSafetyAnalysis.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Bitcode/BitcodeWriterPass.h"
#include "llvm/Support/Caching.h"
#include "llvm/CodeGen/RegAllocRegistry.h"
#include "llvm/CodeGen/SchedulerRegistry.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ModuleSummaryIndex.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IRPrinter/IRPrintingPasses.h"
#include "llvm/LTO/LTOBackend.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/TargetParser/SubtargetFeature.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Support/BuryPointer.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/TimeProfiler.h"
#include "llvm/Support/Timer.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/IPO/LowerTypeTests.h"
#include "llvm/Transforms/IPO/ThinLTOBitcodeWriter.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Instrumentation/AddressSanitizer.h"
#include "llvm/Transforms/Instrumentation/BoundsChecking.h"
#include "llvm/Transforms/Instrumentation/GCOVProfiler.h"
#include "llvm/Transforms/Instrumentation/HWAddressSanitizer.h"
#include "llvm/Transforms/Instrumentation/InstrProfiling.h"
#include "llvm/Transforms/Instrumentation/MemorySanitizer.h"
#include "llvm/Transforms/Instrumentation/SanitizerCoverage.h"
#include "llvm/Transforms/Instrumentation/ThreadSanitizer.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Utils/CanonicalizeAliases.h"
#include "llvm/Transforms/Utils/EntryExitInstrumenter.h"
#include "llvm/Transforms/Utils/NameAnonGlobals.h"
#include "llvm/Transforms/Utils/SymbolRewriter.h"
#include <memory>

using namespace fly;

#define HANDLE_EXTENSION(Ext)                                                  \
  llvm::PassPluginLibraryInfo get##Ext##PluginInfo();
#include "llvm/Support/Extension.def"

namespace {

    // Default filename used for profile generation.
    static constexpr llvm::StringLiteral DefaultProfileGenName = "default_%m.profraw";

    class EmitAssemblyHelper {

        DiagnosticsEngine &Diags;

        const CodeGenOptions &CodeGenOpts;

        const fly::TargetOptions &TargetOpts;

        const bool FrontendTimesIsEnabled;

        llvm::Module *TheModule;

        llvm::Timer CodeGenerationTime;

        std::unique_ptr<raw_pwrite_stream> OS;

        llvm::TargetIRAnalysis getTargetIRAnalysis() const {
            if (TM)
                return TM->getTargetIRAnalysis();

            return llvm::TargetIRAnalysis();
        }

        /// Generates the TargetMachine.
        /// Leaves TM unchanged if it is unable to create the target machine.
        /// Some of our fly tests specify triples which are not built
        /// into fly. This is okay because these tests check the generated
        /// IR, and they require DataLayout which depends on the triple.
        /// In this case, we allow this method to fail and not report an error.
        /// When MustCreateTM is used, we print an error if we are unable to load
        /// the requested target.
        void CreateTargetMachine(bool MustCreateTM);

        /// Add passes necessary to emit assembly or LLVM IR using the legacy
        /// code generation pass manager (still needed for machine code emission).
        ///
        /// \return True on success.
        bool AddEmitPasses(llvm::legacy::PassManager &CodeGenPasses, BackendActionKind Action,
                           raw_pwrite_stream &OS, raw_pwrite_stream *DwoOS);

        std::unique_ptr<llvm::ToolOutputFile> openOutputFile(StringRef Path) {
            std::error_code EC;
            auto F = std::make_unique<llvm::ToolOutputFile>(Path, EC, llvm::sys::fs::OF_None);
            if (EC) {
                Diags.Report(diag::err_fe_unable_to_open_output) << Path << EC.message();
                F.reset();
            }
            return F;
        }

    public:
        EmitAssemblyHelper(DiagnosticsEngine &Diags,
                           const CodeGenOptions &CGOpts,
                           const fly::TargetOptions &TOpts,
                           bool TimePasses,
                           llvm::Module *M)
        : Diags(Diags), CodeGenOpts(CGOpts),
                  TargetOpts(TOpts), FrontendTimesIsEnabled(TimePasses), TheModule(M),
                  CodeGenerationTime("GenStmt", "Code Generation Time") {
            llvm::TimePassesIsEnabled = TimePasses;
        }

        ~EmitAssemblyHelper() {
            if (CodeGenOpts.DisableFree)
                llvm::BuryPointer(std::move(TM));
        }

        std::unique_ptr<llvm::TargetMachine> TM;

        void EmitAssembly(BackendActionKind Action,
                          std::unique_ptr<raw_pwrite_stream> OS);
    };
}

static llvm::SanitizerCoverageOptions getSancovOptsFromCGOpts(const CodeGenOptions &CGOpts) {
    llvm::SanitizerCoverageOptions Opts;
    Opts.CoverageType =
            static_cast<llvm::SanitizerCoverageOptions::Type>(CGOpts.SanitizeCoverageType);
    Opts.IndirectCalls = CGOpts.SanitizeCoverageIndirectCalls;
    Opts.TraceBB = CGOpts.SanitizeCoverageTraceBB;
    Opts.TraceCmp = CGOpts.SanitizeCoverageTraceCmp;
    Opts.TraceDiv = CGOpts.SanitizeCoverageTraceDiv;
    Opts.TraceGep = CGOpts.SanitizeCoverageTraceGep;
    Opts.Use8bitCounters = CGOpts.SanitizeCoverage8bitCounters;
    Opts.TracePC = CGOpts.SanitizeCoverageTracePC;
    Opts.TracePCGuard = CGOpts.SanitizeCoverageTracePCGuard;
    Opts.NoPrune = CGOpts.SanitizeCoverageNoPrune;
    Opts.Inline8bitCounters = CGOpts.SanitizeCoverageInline8bitCounters;
    Opts.InlineBoolFlag = CGOpts.SanitizeCoverageInlineBoolFlag;
    Opts.PCTable = CGOpts.SanitizeCoveragePCTable;
    Opts.StackDepth = CGOpts.SanitizeCoverageStackDepth;
    return Opts;
}

// Check if ASan should use GC-friendly instrumentation for globals.
// First of all, there is no point if -fdata-sections is off (expect for MachO,
// where this is not a factor). Also, on ELF this feature requires an assembler
// extension that only works with -integrated-as at the moment.
static bool asanUseGlobalsGC(const llvm::Triple &T, const CodeGenOptions &CGOpts) {
    if (!CGOpts.SanitizeAddressGlobalsDeadStripping)
        return false;
    switch (T.getObjectFormat()) {
        case llvm::Triple::MachO:
        case llvm::Triple::COFF:
            return true;
        case llvm::Triple::ELF:
            return CGOpts.DataSections && !CGOpts.DisableIntegratedAS;
        case llvm::Triple::XCOFF:
            llvm::report_fatal_error("ASan not implemented for XCOFF.");
        case llvm::Triple::Wasm:
        case llvm::Triple::DXContainer:
        case llvm::Triple::GOFF:
        case llvm::Triple::SPIRV:
        case llvm::Triple::UnknownObjectFormat:
            break;
    }
    return false;
}

static llvm::TargetLibraryInfoImpl *createTLII(llvm::Triple &TargetTriple, const CodeGenOptions &CodeGenOpts) {
    llvm::TargetLibraryInfoImpl *TLII = new llvm::TargetLibraryInfoImpl(TargetTriple);

    switch (CodeGenOpts.getVecLib()) {
        case CodeGenOptions::Accelerate:
            TLII->addVectorizableFunctionsFromVecLib(llvm::TargetLibraryInfoImpl::Accelerate,
                                                     TargetTriple);
            break;
        case CodeGenOptions::MASSV:
            TLII->addVectorizableFunctionsFromVecLib(llvm::TargetLibraryInfoImpl::MASSV,
                                                     TargetTriple);
            break;
        case CodeGenOptions::SVML:
            TLII->addVectorizableFunctionsFromVecLib(llvm::TargetLibraryInfoImpl::SVML,
                                                     TargetTriple);
            break;
        default:
            break;
    }
    return TLII;
}

static void setCommandLineOpts(const CodeGenOptions &CodeGenOpts) {
    SmallVector<const char *, 16> BackendArgs;
    BackendArgs.push_back("fly"); // Fake program name.
    if (!CodeGenOpts.DebugPass.empty()) {
        BackendArgs.push_back("-debug-pass");
        BackendArgs.push_back(CodeGenOpts.DebugPass.c_str());
    }
    if (!CodeGenOpts.LimitFloatPrecision.empty()) {
        BackendArgs.push_back("-limit-float-precision");
        BackendArgs.push_back(CodeGenOpts.LimitFloatPrecision.c_str());
    }
    BackendArgs.push_back(nullptr);
    llvm::cl::ParseCommandLineOptions(BackendArgs.size() - 1,
                                      BackendArgs.data());
}

static llvm::CodeGenOptLevel getCGOptLevel(const CodeGenOptions &CodeGenOpts) {
    switch (CodeGenOpts.OptimizationLevel) {
        default:
            llvm_unreachable("Invalid optimization level!");
        case 0:
            return llvm::CodeGenOptLevel::None;
        case 1:
            return llvm::CodeGenOptLevel::Less;
        case 2:
            return llvm::CodeGenOptLevel::Default; // O2/Os/Oz
        case 3:
            return llvm::CodeGenOptLevel::Aggressive;
    }
}

static std::optional<llvm::CodeModel::Model> getCodeModel(const CodeGenOptions &CodeGenOpts) {
    unsigned CodeModel = llvm::StringSwitch<unsigned>(CodeGenOpts.CodeModel)
            .Case("tiny", llvm::CodeModel::Tiny)
            .Case("small", llvm::CodeModel::Small)
            .Case("kernel", llvm::CodeModel::Kernel)
            .Case("medium", llvm::CodeModel::Medium)
            .Case("large", llvm::CodeModel::Large)
            .Case("default", ~1u)
            .Default(~0u);
    assert(CodeModel != ~0u && "invalid code model!");
    if (CodeModel == ~1u)
        return std::nullopt;
    return static_cast<llvm::CodeModel::Model>(CodeModel);
}

static llvm::CodeGenFileType getCodeGenFileType(BackendActionKind Action) {
    if (Action == Backend_EmitObj)
        return llvm::CodeGenFileType::ObjectFile;
    else if (Action == Backend_EmitNothing)
        return llvm::CodeGenFileType::Null;
    else {
        assert(Action == Backend_EmitAssembly && "Invalid action!");
        return llvm::CodeGenFileType::AssemblyFile;
    }
}

static void initTargetOptions(DiagnosticsEngine &Diags,
                              llvm::TargetOptions &Options,
                              const CodeGenOptions &CodeGenOpts,
                              const fly::TargetOptions &TargetOpts) {
    Options.ThreadModel =
            llvm::StringSwitch<llvm::ThreadModel::Model>(CodeGenOpts.ThreadModel)
                    .Case("posix", llvm::ThreadModel::POSIX)
                    .Case("single", llvm::ThreadModel::Single);

    // Set float ABI type.
    assert((CodeGenOpts.FloatABI == "soft" || CodeGenOpts.FloatABI == "softfp" ||
            CodeGenOpts.FloatABI == "hard" || CodeGenOpts.FloatABI.empty()) &&
           "Invalid Floating Point ABI!");
    Options.FloatABIType =
            llvm::StringSwitch<llvm::FloatABI::ABIType>(CodeGenOpts.FloatABI)
                    .Case("soft", llvm::FloatABI::Soft)
                    .Case("softfp", llvm::FloatABI::Soft)
                    .Case("hard", llvm::FloatABI::Hard)
                    .Default(llvm::FloatABI::Default);

    // TODO Set FP fusion mode.
    Options.AllowFPOpFusion = llvm::FPOpFusion::Standard;
    // Options.AllowFPOpFusion = llvm::FPOpFusion::Fast;

    Options.UseInitArray = CodeGenOpts.UseInitArray;
    Options.DisableIntegratedAS = CodeGenOpts.DisableIntegratedAS;
    // Note: CompressDebugSections and RelaxELFRelocations were removed from
    // TargetOptions in LLVM 17+; those settings are now handled elsewhere.

    // Set EABI version.
    Options.EABIVersion = TargetOpts.EABIVersion;

    // TODO chose ExceptionHandling
    Options.ExceptionModel = llvm::ExceptionHandling::SjLj;
//    Options.ExceptionModel = llvm::ExceptionHandling::WinEH;
//    Options.ExceptionModel = llvm::ExceptionHandling::DwarfCFI;
//    Options.ExceptionModel = llvm::ExceptionHandling::Wasm;

    // TODO chose language behavior
    Options.NoInfsFPMath = 1;
    Options.NoNaNsFPMath = 1;
    Options.NoZerosInBSS = CodeGenOpts.NoZeroInitializedInBSS;
    Options.UnsafeFPMath = 1;
    // Note: StackAlignmentOverride was removed from TargetOptions in LLVM 17+.

    Options.BBSections =
            llvm::StringSwitch<llvm::BasicBlockSection>(CodeGenOpts.BBSections)
                    .Case("all", llvm::BasicBlockSection::All)
                    .StartsWith("list=", llvm::BasicBlockSection::List)
                    .Case("none", llvm::BasicBlockSection::None)
                    .Default(llvm::BasicBlockSection::None);

    if (Options.BBSections == llvm::BasicBlockSection::List) {
        llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> MBOrErr =
                llvm::MemoryBuffer::getFile(CodeGenOpts.BBSections.substr(5));
        if (!MBOrErr)
            Diags.Report(diag::err_fe_unable_to_load_basic_block_sections_file) << MBOrErr.getError().message();
        else
            Options.BBSectionsFuncListBuf = std::move(*MBOrErr);
    }

    Options.FunctionSections = CodeGenOpts.FunctionSections;
    Options.DataSections = CodeGenOpts.DataSections;
    Options.UniqueSectionNames = CodeGenOpts.UniqueSectionNames;
    Options.UniqueBasicBlockSectionNames =
            CodeGenOpts.UniqueBasicBlockSectionNames;
    Options.TLSSize = CodeGenOpts.TLSSize;
    Options.EmulatedTLS = CodeGenOpts.EmulatedTLS;
    // Note: ExplicitEmulatedTLS was removed from TargetOptions in LLVM 17+.
    Options.DebuggerTuning = CodeGenOpts.getDebuggerTuning();
    Options.EmitStackSizeSection = CodeGenOpts.StackSizeSection;
    Options.EmitAddrsig = CodeGenOpts.Addrsig;
    Options.ForceDwarfFrameSection = CodeGenOpts.ForceDwarfFrameSection;
    Options.EmitCallSiteInfo = CodeGenOpts.EmitCallSiteInfo;
    // Note: XRayOmitFunctionIndex was renamed to XRayFunctionIndex (inverted).
    Options.XRayFunctionIndex = !CodeGenOpts.XRayOmitFunctionIndex;

    Options.MCOptions.SplitDwarfFile = CodeGenOpts.SplitDwarfFile;
    Options.MCOptions.MCRelaxAll = CodeGenOpts.RelaxAll;
    Options.MCOptions.MCSaveTempLabels = CodeGenOpts.SaveTempLabels;
    // MCUseDwarfDirectory changed from bool to DwarfDirectory enum in LLVM 17+.
    Options.MCOptions.MCUseDwarfDirectory = CodeGenOpts.NoDwarfDirectoryAsm
        ? llvm::MCTargetOptions::DisableDwarfDirectory
        : llvm::MCTargetOptions::DefaultDwarfDirectory;
    Options.MCOptions.MCNoExecStack = CodeGenOpts.NoExecStack;
    Options.MCOptions.MCIncrementalLinkerCompatible =
            CodeGenOpts.IncrementalLinkerCompatible;
    Options.MCOptions.MCFatalWarnings = CodeGenOpts.FatalWarnings;
    Options.MCOptions.MCNoWarn = CodeGenOpts.NoWarn;
    Options.MCOptions.AsmVerbose = CodeGenOpts.AsmVerbose;
    Options.MCOptions.PreserveAsmComments = CodeGenOpts.PreserveAsmComments;
    Options.MCOptions.ABIName = TargetOpts.ABI;
    // TODO add paths for dependencies
//    for (const auto &Entry : HSOpts.UserEntries)
//        if (!Entry.IsFramework &&
//            (Entry.Group == frontend::IncludeDirGroup::Quoted ||
//             Entry.Group == frontend::IncludeDirGroup::Angled ||
//             Entry.Group == frontend::IncludeDirGroup::System))
//            Options.MCOptions.IASSearchPaths.push_back(
//                    Entry.IgnoreSysRoot ? Entry.Path : HSOpts.Sysroot + Entry.Path);
    if (CodeGenOpts.Argv0)
        Options.MCOptions.Argv0 = CodeGenOpts.Argv0;
    // Note: MCOptions.CommandLineArgs was removed in LLVM 17+.
}

static std::optional<llvm::GCOVOptions> getGCOVOptions(const CodeGenOptions &CodeGenOpts) {
    if (CodeGenOpts.DisableGCov)
        return std::nullopt;
    if (!CodeGenOpts.EmitGcovArcs && !CodeGenOpts.EmitGcovNotes)
        return std::nullopt;
    // Not using 'GCOVOptions::getDefault' allows us to avoid exiting if
    // LLVM's -default-gcov-version flag is set to something invalid.
    llvm::GCOVOptions Options;
    Options.EmitNotes = CodeGenOpts.EmitGcovNotes;
    Options.EmitData = CodeGenOpts.EmitGcovArcs;
    llvm::copy(CodeGenOpts.CoverageVersion, std::begin(Options.Version));
    Options.NoRedZone = CodeGenOpts.DisableRedZone;
    Options.Filter = CodeGenOpts.ProfileFilterFiles;
    Options.Exclude = CodeGenOpts.ProfileExcludeFiles;
    return Options;
}

static std::optional<llvm::InstrProfOptions> getInstrProfOptions(const CodeGenOptions &CodeGenOpts) {
    if (!CodeGenOpts.hasProfileClangInstr())
        return std::nullopt;
    llvm::InstrProfOptions Options;
    Options.NoRedZone = CodeGenOpts.DisableRedZone;
    Options.InstrProfileOutput = CodeGenOpts.InstrProfileOutput;

    // TODO: Surface the option to emit atomic profile counter increments at
    // the driver level.
    Options.Atomic = false;
    return Options;
}

void EmitAssemblyHelper::CreateTargetMachine(bool MustCreateTM) {
    // Create the TargetMachine for generating code.
    std::string Error;
    std::string Triple = TheModule->getTargetTriple();
    const llvm::Target *TheTarget = llvm::TargetRegistry::lookupTarget(Triple, Error);
    if (!TheTarget) {
        if (MustCreateTM)
            Diags.Report(diag::err_fe_unable_to_create_target) << Error;
        return;
    }

    std::optional<llvm::CodeModel::Model> CM = getCodeModel(CodeGenOpts);
    std::string FeaturesStr =
            llvm::join(TargetOpts.Features.begin(), TargetOpts.Features.end(), ",");
    llvm::Reloc::Model RM = CodeGenOpts.RelocationModel;
    llvm::CodeGenOptLevel OptLevel = getCGOptLevel(CodeGenOpts);

    llvm::TargetOptions Options;
    initTargetOptions(Diags, Options, CodeGenOpts, TargetOpts);
    TM.reset(TheTarget->createTargetMachine(Triple, TargetOpts.CPU, FeaturesStr,
                                            Options, RM, CM, OptLevel));
}

bool EmitAssemblyHelper::AddEmitPasses(llvm::legacy::PassManager &CodeGenPasses,
                                       BackendActionKind Action,
                                       raw_pwrite_stream &OS,
                                       raw_pwrite_stream *DwoOS) {
    // Add LibraryInfo.
    llvm::Triple TargetTriple(TheModule->getTargetTriple());
    std::unique_ptr<llvm::TargetLibraryInfoImpl> TLII(
            createTLII(TargetTriple, CodeGenOpts));
    CodeGenPasses.add(new llvm::TargetLibraryInfoWrapperPass(*TLII));

    // Normal mode, emit a .s or .o file by running the code generator. Note,
    // this also adds codegenerator level optimization passes.
    llvm::CodeGenFileType CGFT = getCodeGenFileType(Action);

    if (TM->addPassesToEmitFile(CodeGenPasses, OS, DwoOS, CGFT,
            /*DisableVerify=*/!CodeGenOpts.VerifyModule)) {
        Diags.Report(diag::err_fe_unable_to_interface_with_target);
        return false;
    }

    return true;
}

static llvm::OptimizationLevel mapToLevel(const CodeGenOptions &Opts) {
    switch (Opts.OptimizationLevel) {
        default:
            llvm_unreachable("Invalid optimization level!");

        case 1:
            return llvm::OptimizationLevel::O1;

        case 2:
            switch (Opts.OptimizeSize) {
                default:
                    llvm_unreachable("Invalid optimization level for size!");

                case 0:
                    return llvm::OptimizationLevel::O2;

                case 1:
                    return llvm::OptimizationLevel::Os;

                case 2:
                    return llvm::OptimizationLevel::Oz;
            }

        case 3:
            return llvm::OptimizationLevel::O3;
    }
}

/// Emit assembly/IR/object using the new pass manager.
void EmitAssemblyHelper::EmitAssembly(BackendActionKind Action,
                                      std::unique_ptr<raw_pwrite_stream> OS) {
    std::unique_ptr<llvm::TimeRegion> TimingRegion;
    if (FrontendTimesIsEnabled)
        TimingRegion = std::make_unique<llvm::TimeRegion>(&CodeGenerationTime);
    setCommandLineOpts(CodeGenOpts);

    bool RequiresCodeGen = (Action != Backend_EmitNothing &&
                            Action != Backend_EmitBC &&
                            Action != Backend_EmitLL);
    CreateTargetMachine(RequiresCodeGen);

    if (RequiresCodeGen && !TM)
        return;
    if (TM)
        TheModule->setDataLayout(TM->createDataLayout());

    std::optional<llvm::PGOOptions> PGOOpt;

    if (CodeGenOpts.hasProfileIRInstr())
        // -fprofile-generate.
        PGOOpt = llvm::PGOOptions(CodeGenOpts.InstrProfileOutput.empty()
                            ? std::string(DefaultProfileGenName)
                            : CodeGenOpts.InstrProfileOutput,
                            /*CSProfileGenFile=*/"",
                            /*ProfileRemappingFile=*/"",
                            /*MemoryProfile=*/"",
                            /*FS=*/nullptr,
                            llvm::PGOOptions::IRInstr,
                            llvm::PGOOptions::NoCSAction,
                            llvm::PGOOptions::ColdFuncOpt::Default,
                            CodeGenOpts.DebugInfoForProfiling);
    else if (CodeGenOpts.hasProfileIRUse()) {
        // -fprofile-use.
        auto CSAction = CodeGenOpts.hasProfileCSIRUse() ? llvm::PGOOptions::CSIRUse
                                                        : llvm::PGOOptions::NoCSAction;
        PGOOpt = llvm::PGOOptions(CodeGenOpts.ProfileInstrumentUsePath,
                            /*CSProfileGenFile=*/"",
                            CodeGenOpts.ProfileRemappingFile,
                            /*MemoryProfile=*/"",
                            /*FS=*/nullptr,
                            llvm::PGOOptions::IRUse,
                            CSAction,
                            llvm::PGOOptions::ColdFuncOpt::Default,
                            CodeGenOpts.DebugInfoForProfiling);
    } else if (!CodeGenOpts.SampleProfileFile.empty())
        // -fprofile-sample-use
        PGOOpt = llvm::PGOOptions(CodeGenOpts.SampleProfileFile,
                           /*CSProfileGenFile=*/"",
                           CodeGenOpts.ProfileRemappingFile,
                           /*MemoryProfile=*/"",
                           /*FS=*/nullptr,
                           llvm::PGOOptions::SampleUse,
                           llvm::PGOOptions::NoCSAction,
                           llvm::PGOOptions::ColdFuncOpt::Default,
                           CodeGenOpts.DebugInfoForProfiling);
    else if (CodeGenOpts.DebugInfoForProfiling)
        // -fdebug-info-for-profiling
        PGOOpt = llvm::PGOOptions(/*ProfileFile=*/"",
                            /*CSProfileGenFile=*/"",
                            /*ProfileRemappingFile=*/"",
                            /*MemoryProfile=*/"",
                            /*FS=*/nullptr,
                            llvm::PGOOptions::NoAction,
                            llvm::PGOOptions::NoCSAction,
                            llvm::PGOOptions::ColdFuncOpt::Default,
                            /*DebugInfoForProfiling=*/true);

    // Check to see if we want to generate a CS profile.
    if (CodeGenOpts.hasProfileCSIRInstr()) {
        assert(!CodeGenOpts.hasProfileCSIRUse() &&
               "Cannot have both CSProfileUse pass and CSProfileGen pass at "
               "the same time");
        if (PGOOpt.has_value()) {
            assert(PGOOpt->Action != llvm::PGOOptions::IRInstr &&
                   PGOOpt->Action != llvm::PGOOptions::SampleUse &&
                   "Cannot run CSProfileGen pass with ProfileGen or SampleUse "
                   " pass");
            PGOOpt->CSProfileGenFile = CodeGenOpts.InstrProfileOutput.empty()
                                       ? std::string(DefaultProfileGenName)
                                       : CodeGenOpts.InstrProfileOutput;
            PGOOpt->CSAction = llvm::PGOOptions::CSIRInstr;
        } else
            PGOOpt = llvm::PGOOptions(/*ProfileFile=*/"",
                                CodeGenOpts.InstrProfileOutput.empty()
                                ? std::string(DefaultProfileGenName)
                                : CodeGenOpts.InstrProfileOutput,
                                /*ProfileRemappingFile=*/"",
                                /*MemoryProfile=*/"",
                                /*FS=*/nullptr,
                                llvm::PGOOptions::NoAction,
                                llvm::PGOOptions::CSIRInstr);
    }

    llvm::PipelineTuningOptions PTO;
    PTO.LoopUnrolling = CodeGenOpts.UnrollLoops;
    // For historical reasons, loop interleaving is set to mirror setting for loop
    // unrolling.
    PTO.LoopInterleaving = CodeGenOpts.UnrollLoops;
    PTO.LoopVectorization = CodeGenOpts.VectorizeLoop;
    PTO.SLPVectorization = CodeGenOpts.VectorizeSLP;
    // Only enable CGProfilePass when using integrated assembler, since
    // non-integrated assemblers don't recognize .cgprofile section.
    PTO.CallGraphProfile = !CodeGenOpts.DisableIntegratedAS;

    llvm::PassInstrumentationCallbacks PIC;
    llvm::StandardInstrumentations SI(TheModule->getContext(),
                                      /*DebugLogging=*/false);
    SI.registerCallbacks(PIC);
    llvm::PassBuilder PB(TM.get(), PTO, PGOOpt, &PIC);

    // Attempt to load pass plugins and register their callbacks with PB.
    for (auto &PluginFN : CodeGenOpts.PassPlugins) {
        auto PassPlugin = llvm::PassPlugin::Load(PluginFN);
        if (PassPlugin) {
            PassPlugin->registerPassBuilderCallbacks(PB);
        } else {
            Diags.Report(diag::err_fe_unable_to_load_plugin)
                    << PluginFN << toString(PassPlugin.takeError());
        }
    }
#define HANDLE_EXTENSION(Ext)                                                  \
  get##Ext##PluginInfo().RegisterPassBuilderCallbacks(PB);
#include "llvm/Support/Extension.def"

    llvm::LoopAnalysisManager LAM;
    llvm::FunctionAnalysisManager FAM;
    llvm::CGSCCAnalysisManager CGAM;
    llvm::ModuleAnalysisManager MAM;

    // Register the AA manager first so that our version is the one used.
    FAM.registerPass([&] { return PB.buildDefaultAAPipeline(); });

    // Register the target library analysis directly and give it a customized
    // preset TLI.
    llvm::Triple TargetTriple(TheModule->getTargetTriple());
    std::unique_ptr<llvm::TargetLibraryInfoImpl> TLII(
            createTLII(TargetTriple, CodeGenOpts));
    FAM.registerPass([&] { return llvm::TargetLibraryAnalysis(*TLII); });

    // Register all the basic analyses with the managers.
    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    llvm::ModulePassManager MPM;

    if (!CodeGenOpts.DisableLLVMPasses) {
        bool IsThinLTO = CodeGenOpts.PrepareForThinLTO;
        bool IsLTO = CodeGenOpts.PrepareForLTO;

        if (CodeGenOpts.OptimizationLevel == 0) {
            // If we reached here with a non-empty index file name, then the index
            // file was empty and we are not performing ThinLTO backend compilation
            // (used in testing in a distributed build environment). Drop any the type
            // test assume sequences inserted for whole program vtables so that
            // codegen doesn't complain.
            if (!CodeGenOpts.ThinLTOIndexFile.empty())
                MPM.addPass(llvm::LowerTypeTestsPass(/*ExportSummary=*/nullptr,
                        /*ImportSummary=*/nullptr,
                        llvm::lowertypetests::DropTestKind::All));

            if (std::optional<llvm::GCOVOptions> Options = getGCOVOptions(CodeGenOpts))
                MPM.addPass(llvm::GCOVProfilerPass(*Options));
            if (std::optional<llvm::InstrProfOptions> Options = getInstrProfOptions(CodeGenOpts))
                MPM.addPass(llvm::InstrProfilingLoweringPass(*Options, false));

            // Build a minimal pipeline based on the semantics required by Fly,
            // which is just that always inlining occurs. Further, disable generating
            // lifetime intrinsics to avoid enabling further optimizations during
            // code generation.
            MPM.addPass(llvm::AlwaysInlinerPass(/*InsertLifetimeIntrinsics=*/false));

            // At -O0, we can still do PGO. Add all the requested passes for
            // instrumentation PGO, if requested.
            if (PGOOpt && (PGOOpt->Action == llvm::PGOOptions::IRInstr ||
                           PGOOpt->Action == llvm::PGOOptions::IRUse))
                PB.addPGOInstrPassesForO0(
                        MPM,
                        /* RunProfileGen */ (PGOOpt->Action == llvm::PGOOptions::IRInstr),
                        /* IsCS */ false,
                        /* AtomicCounterUpdate */ false,
                        PGOOpt->ProfileFile,
                        PGOOpt->ProfileRemappingFile,
                        /*FS=*/nullptr);

            // At -O0 we directly run necessary sanitizer passes.
            // TODO
//            if (LangOpts.Sanitize.has(SanitizerKind::LocalBounds))
//                MPM.addPass(createModuleToFunctionPassAdaptor(BoundsCheckingPass()));

            // Lastly, add semantically necessary passes for LTO.
            if (IsLTO || IsThinLTO) {
                MPM.addPass(llvm::CanonicalizeAliasesPass());
                MPM.addPass(llvm::NameAnonGlobalPass());
            }
        } else {
            // Map our optimization levels into one of the distinct levels used to
            // configure the pipeline.
            llvm::OptimizationLevel Level = mapToLevel(CodeGenOpts);

            // If we reached here with a non-empty index file name, then the index
            // file was empty and we are not performing ThinLTO backend compilation
            // (used in testing in a distributed build environment). Drop any the type
            // test assume sequences inserted for whole program vtables so that
            // codegen doesn't complain.
            if (!CodeGenOpts.ThinLTOIndexFile.empty())
                PB.registerPipelineStartEPCallback(
                        [](llvm::ModulePassManager &MPM, llvm::OptimizationLevel) {
                    MPM.addPass(llvm::LowerTypeTestsPass(/*ExportSummary=*/nullptr,
                            /*ImportSummary=*/nullptr,
                            llvm::lowertypetests::DropTestKind::All));
                });

            PB.registerPipelineStartEPCallback(
                    [](llvm::ModulePassManager &MPM, llvm::OptimizationLevel) {
                MPM.addPass(llvm::createModuleToFunctionPassAdaptor(
                        llvm::EntryExitInstrumenterPass(/*PostInlining=*/false)));
            });

            // Register callbacks to schedule sanitizer passes at the appropriate
            // part of the pipeline.
            // TODO
//            if (LangOpts.Sanitize.has(SanitizerKind::LocalBounds))
//                PB.registerScalarOptimizerLateEPCallback(
//                        [](FunctionPassManager &FPM, OptimizationLevel Level) {
//                            FPM.addPass(BoundsCheckingPass());
//                        });

            if (CodeGenOpts.SanitizeCoverageType ||
                CodeGenOpts.SanitizeCoverageIndirectCalls ||
                CodeGenOpts.SanitizeCoverageTraceCmp) {
                PB.registerOptimizerLastEPCallback(
                        [this](llvm::ModulePassManager &MPM,
                               llvm::OptimizationLevel Level,
                               llvm::ThinOrFullLTOPhase) {
                            auto SancovOpts = getSancovOptsFromCGOpts(CodeGenOpts);
                            MPM.addPass(llvm::SanitizerCoveragePass(
                                    SancovOpts, CodeGenOpts.SanitizeCoverageAllowlistFiles,
                                    CodeGenOpts.SanitizeCoverageBlocklistFiles));
                        });
            }

            // TODO
//            if (LangOpts.Sanitize.has(SanitizerKind::Memory)) { ... }
//            if (LangOpts.Sanitize.has(SanitizerKind::Thread)) { ... }
//            if (LangOpts.Sanitize.has(SanitizerKind::Address)) { ... }

            if (std::optional<llvm::GCOVOptions> Options = getGCOVOptions(CodeGenOpts))
                PB.registerPipelineStartEPCallback(
                        [Options](llvm::ModulePassManager &MPM, llvm::OptimizationLevel) {
                    MPM.addPass(llvm::GCOVProfilerPass(*Options));
                });

            // TODO
//            if (std::optional<InstrProfOptions> Options = getInstrProfOptions(CodeGenOpts))
//                PB.registerPipelineStartEPCallback(...);

            if (IsThinLTO) {
                MPM = PB.buildThinLTOPreLinkDefaultPipeline(Level);
                MPM.addPass(llvm::CanonicalizeAliasesPass());
                MPM.addPass(llvm::NameAnonGlobalPass());
            } else if (IsLTO) {
                MPM = PB.buildLTOPreLinkDefaultPipeline(Level);
                MPM.addPass(llvm::CanonicalizeAliasesPass());
                MPM.addPass(llvm::NameAnonGlobalPass());
            } else {
                MPM = PB.buildPerModuleDefaultPipeline(Level);
            }
        }

        // TODO
//        if (LangOpts.Sanitize.has(SanitizerKind::HWAddress)) { ... }
//        if (LangOpts.Sanitize.has(SanitizerKind::KernelHWAddress)) { ... }
    }

    // We still use the legacy pass manager to do code generation (machine code
    // emission). We create that pass manager here and use it as needed below.
    llvm::legacy::PassManager CodeGenPasses;
    bool NeedCodeGen = false;
    std::unique_ptr<llvm::ToolOutputFile> ThinLinkOS, DwoOS;

    // Append any output we need to the pass manager.
    switch (Action) {
        case Backend_EmitNothing:
            break;

        case Backend_EmitBC:
            if (CodeGenOpts.PrepareForThinLTO && !CodeGenOpts.DisableLLVMPasses) {
                if (!CodeGenOpts.ThinLinkBitcodeFile.empty()) {
                    ThinLinkOS = openOutputFile(CodeGenOpts.ThinLinkBitcodeFile);
                    if (!ThinLinkOS)
                        return;
                }
                TheModule->addModuleFlag(llvm::Module::Error, "EnableSplitLTOUnit",
                                         CodeGenOpts.EnableSplitLTOUnit);
                MPM.addPass(llvm::ThinLTOBitcodeWriterPass(*OS, ThinLinkOS ? &ThinLinkOS->os()
                                                                     : nullptr));
            } else {
                // Emit a module summary by default for Regular LTO except for ld64
                // targets
                bool EmitLTOSummary =
                        (CodeGenOpts.PrepareForLTO &&
                         !CodeGenOpts.DisableLLVMPasses &&
                         llvm::Triple(TheModule->getTargetTriple()).getVendor() !=
                         llvm::Triple::Apple);
                if (EmitLTOSummary) {
                    if (!TheModule->getModuleFlag("ThinLTO"))
                        TheModule->addModuleFlag(llvm::Module::Error, "ThinLTO", uint32_t(0));
                    TheModule->addModuleFlag(llvm::Module::Error, "EnableSplitLTOUnit",
                                             uint32_t(1));
                }
                MPM.addPass(llvm::BitcodeWriterPass(*OS, CodeGenOpts.EmitLLVMUseLists, EmitLTOSummary));
            }
            break;

        case Backend_EmitLL:
            MPM.addPass(llvm::PrintModulePass(*OS, "", CodeGenOpts.EmitLLVMUseLists));
            break;

        case Backend_EmitAssembly:
        case Backend_EmitObj:
            NeedCodeGen = true;
            CodeGenPasses.add(
                    createTargetTransformInfoWrapperPass(getTargetIRAnalysis()));
            if (!CodeGenOpts.SplitDwarfOutput.empty()) {
                DwoOS = openOutputFile(CodeGenOpts.SplitDwarfOutput);
                if (!DwoOS)
                    return;
            }
            if (!AddEmitPasses(CodeGenPasses, Action, *OS,
                               DwoOS ? &DwoOS->os() : nullptr))
                // FIXME: Should we handle this error differently?
                return;
            break;
    }

    // Before executing passes, print the final values of the LLVM options.
    llvm::cl::PrintOptionValues();

    // Now that we have all of the passes ready, run them.
    {
        llvm::PrettyStackTraceString CrashInfo("Optimizer");
        MPM.run(*TheModule, MAM);
    }

    // Now if needed, run the legacy PM for codegen.
    if (NeedCodeGen) {
        llvm::PrettyStackTraceString CrashInfo("Code generation");
        CodeGenPasses.run(*TheModule);
    }

    if (ThinLinkOS)
        ThinLinkOS->keep();
    if (DwoOS)
        DwoOS->keep();
}

Expected<llvm::BitcodeModule> fly::FindThinLTOModule(llvm::MemoryBufferRef MBRef) {
    Expected<std::vector<llvm::BitcodeModule>> BMsOrErr = llvm::getBitcodeModuleList(MBRef);
    if (!BMsOrErr)
        return BMsOrErr.takeError();

    // The bitcode file may contain multiple modules, we want the one that is
    // marked as being the ThinLTO module.
    if (const llvm::BitcodeModule *Bm = FindThinLTOModule(*BMsOrErr))
        return *Bm;

    return llvm::make_error<llvm::StringError>("Could not find module summary",
                                   llvm::inconvertibleErrorCode());
}

llvm::BitcodeModule *fly::FindThinLTOModule(MutableArrayRef<llvm::BitcodeModule> BMs) {
    for (llvm::BitcodeModule &BM : BMs) {
        Expected<llvm::BitcodeLTOInfo> LTOInfo = BM.getLTOInfo();
        if (LTOInfo && LTOInfo->IsThinLTO)
            return &BM;
    }
    return nullptr;
}

static void runThinLTOBackend(DiagnosticsEngine &Diags, llvm::ModuleSummaryIndex *CombinedIndex, llvm::Module *M,
                              const CodeGenOptions &CGOpts,
                              const fly::TargetOptions &TOpts,
                              std::unique_ptr<raw_pwrite_stream> OS, std::string SampleProfile,
                              std::string ProfileRemapping, BackendActionKind Action) {
    llvm::StringMap<llvm::DenseMap<llvm::GlobalValue::GUID, llvm::GlobalValueSummary *>>
            ModuleToDefinedGVSummaries;
    CombinedIndex->collectDefinedGVSummariesPerModule(ModuleToDefinedGVSummaries);

    setCommandLineOpts(CGOpts);

    // Build the import list using the LLVM-provided helper, which handles the
    // new ImportIDTable-based ImportMapTy in LLVM 17+.
    llvm::FunctionImporter::ImportListsTy ImportLists;
    llvm::lto::initImportList(*M, *CombinedIndex,
                              ImportLists[M->getModuleIdentifier()]);
    const llvm::FunctionImporter::ImportMapTy &ImportList =
            ImportLists.lookup(M->getModuleIdentifier());

    // AddStreamFn returns a CachedFileStream wrapping our output stream.
    // Only called once (Task=0), so we move the OS on first call.
    auto AddStream = [&](unsigned Task, const llvm::Twine &) -> llvm::Expected<std::unique_ptr<llvm::CachedFileStream>> {
        return std::make_unique<llvm::CachedFileStream>(std::move(OS));
    };
    llvm::lto::Config Conf;
    if (CGOpts.SaveTempsFilePrefix != "") {
        if (llvm::Error E = Conf.addSaveTemps(CGOpts.SaveTempsFilePrefix + ".",
                /* UseInputModulePath */ false)) {
            handleAllErrors(std::move(E), [&](llvm::ErrorInfoBase &EIB) {
                llvm::errs() << "Error setting up ThinLTO save-temps: " << EIB.message()
                       << '\n';
            });
        }
    }
    Conf.CPU = TOpts.CPU;
    Conf.CodeModel = getCodeModel(CGOpts);
    Conf.MAttrs = TOpts.Features;
    Conf.RelocModel = CGOpts.RelocationModel;
    Conf.CGOptLevel = getCGOptLevel(CGOpts);
    Conf.OptLevel = CGOpts.OptimizationLevel;
    initTargetOptions(Diags, Conf.Options, CGOpts, TOpts);
    Conf.SampleProfile = std::move(SampleProfile);
    Conf.PTO.LoopUnrolling = CGOpts.UnrollLoops;
    // For historical reasons, loop interleaving is set to mirror setting for loop
    // unrolling.
    Conf.PTO.LoopInterleaving = CGOpts.UnrollLoops;
    Conf.PTO.LoopVectorization = CGOpts.VectorizeLoop;
    Conf.PTO.SLPVectorization = CGOpts.VectorizeSLP;
    // Only enable CGProfilePass when using integrated assembler, since
    // non-integrated assemblers don't recognize .cgprofile section.
    Conf.PTO.CallGraphProfile = !CGOpts.DisableIntegratedAS;

    // Context sensitive profile.
    if (CGOpts.hasProfileCSIRInstr()) {
        Conf.RunCSIRInstr = true;
        Conf.CSIRProfile = std::move(CGOpts.InstrProfileOutput);
    } else if (CGOpts.hasProfileCSIRUse()) {
        Conf.RunCSIRInstr = false;
        Conf.CSIRProfile = std::move(CGOpts.ProfileInstrumentUsePath);
    }

    Conf.ProfileRemapping = std::move(ProfileRemapping);
    // DebugPassManager was removed from CodeGenOptions; hardcode false.
    Conf.DebugPassManager = false;
    Conf.RemarksWithHotness = CGOpts.DiagnosticsWithHotness;
    Conf.RemarksFilename = CGOpts.OptRecordFile;
    Conf.RemarksPasses = CGOpts.OptRecordPasses;
    Conf.RemarksFormat = CGOpts.OptRecordFormat;
    Conf.SplitDwarfFile = CGOpts.SplitDwarfFile;
    Conf.SplitDwarfOutput = CGOpts.SplitDwarfOutput;
    switch (Action) {
        case Backend_EmitNothing:
            Conf.PreCodeGenModuleHook = [](size_t Task, const llvm::Module &Mod) {
                return false;
            };
            break;
        case Backend_EmitLL:
            Conf.PreCodeGenModuleHook = [&](size_t Task, const llvm::Module &Mod) {
                M->print(*OS, nullptr, CGOpts.EmitLLVMUseLists);
                return false;
            };
            break;
        case Backend_EmitBC:
            Conf.PreCodeGenModuleHook = [&](size_t Task, const llvm::Module &Mod) {
                llvm::WriteBitcodeToFile(*M, *OS, CGOpts.EmitLLVMUseLists);
                return false;
            };
            break;
        default:
            Conf.CGFileType = getCodeGenFileType(Action);
            break;
    }
    if (llvm::Error E = llvm::lto::thinBackend(
            Conf, /*Task=*/0, AddStream, *M, *CombinedIndex, ImportList,
            ModuleToDefinedGVSummaries[M->getModuleIdentifier()],
            /*ModuleMap=*/nullptr, /*CodeGenOnly=*/false)) {
        llvm::handleAllErrors(std::move(E), [&](llvm::ErrorInfoBase &EIB) {
            llvm::errs() << "Error running ThinLTO backend: " << EIB.message() << '\n';
        });
    }
}

void fly::EmitBackendOutput(DiagnosticsEngine &Diags,
                            const CodeGenOptions &CGOpts,
                            const fly::TargetOptions &TOpts,
                            bool TimePasses,
                            const llvm::DataLayout &TDesc,
                            llvm::Module *M,
                            BackendActionKind Action,
                            std::unique_ptr<raw_pwrite_stream> OS) {

    llvm::TimeTraceScope TimeScope("Backend");

    std::unique_ptr<llvm::Module> EmptyModule;
    if (!CGOpts.ThinLTOIndexFile.empty()) {
        // If we are performing a ThinLTO importing compile, load the function index
        // into memory and pass it into runThinLTOBackend, which will run the
        // function importer and invoke LTO passes.
        Expected<std::unique_ptr<llvm::ModuleSummaryIndex>> IndexOrErr =
                llvm::getModuleSummaryIndexForFile(CGOpts.ThinLTOIndexFile,
                        /*IgnoreEmptyThinLTOIndexFile*/true);
        if (!IndexOrErr) {
            logAllUnhandledErrors(IndexOrErr.takeError(), llvm::errs(),
                                  "Error loading index file '" +
                                  CGOpts.ThinLTOIndexFile + "': ");
            return;
        }
        std::unique_ptr<llvm::ModuleSummaryIndex> CombinedIndex = std::move(*IndexOrErr);
        // A null CombinedIndex means we should skip ThinLTO compilation
        // (LLVM will optionally ignore empty index files, returning null instead
        // of an error).
        if (CombinedIndex) {
            if (!CombinedIndex->skipModuleByDistributedBackend()) {
                runThinLTOBackend(Diags, CombinedIndex.get(), M, CGOpts,
                                  TOpts, std::move(OS), CGOpts.SampleProfileFile,
                                  CGOpts.ProfileRemappingFile, Action);
                return;
            }
            // Distributed indexing detected that nothing from the module is needed
            // for the final linking. So we can skip the compilation. We sill need to
            // output an empty object file to make sure that a linker does not fail
            // trying to read it. Also for some features, like CFI, we must skip
            // the compilation as CombinedIndex does not contain all required
            // information.
            EmptyModule = std::make_unique<llvm::Module>("empty", M->getContext());
            EmptyModule->setTargetTriple(M->getTargetTriple());
            M = EmptyModule.get();
        }
    }

    EmitAssemblyHelper AsmHelper(Diags, CGOpts, TOpts, TimePasses, M);
    AsmHelper.EmitAssembly(Action, std::move(OS));

    // Verify fly's TargetInfo DataLayout against the LLVM TargetMachine's
    // DataLayout.
    if (AsmHelper.TM) {
        std::string DLDesc = M->getDataLayout().getStringRepresentation();
        if (DLDesc != TDesc.getStringRepresentation()) {
            unsigned DiagID = Diags.getCustomDiagID(
                    DiagnosticsEngine::Error, "backend data layout '%0' does not match "
                                              "expected target description '%1'");
            Diags.Report(DiagID) << DLDesc << TDesc.getStringRepresentation();
        }
    }
}

// With -fembed-bitcode, save a copy of the llvm IR as data in the
// __LLVM,__bitcode section.
void fly::EmbedBitcode(llvm::Module *M, const CodeGenOptions &CGOpts, llvm::MemoryBufferRef Buf) {
    if (CGOpts.getEmbedBitcode() == CodeGenOptions::Embed_Off)
        return;
    llvm::embedBitcodeInModule(
            *M, Buf, CGOpts.getEmbedBitcode() != CodeGenOptions::Embed_Marker,
            CGOpts.getEmbedBitcode() != CodeGenOptions::Embed_Bitcode,
            CGOpts.CmdArgs);
}
