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
#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/StackSafetyAnalysis.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Bitcode/BitcodeWriterPass.h"
#include "llvm/CodeGen/RegAllocRegistry.h"
#include "llvm/CodeGen/SchedulerRegistry.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ModuleSummaryIndex.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/LTO/LTOBackend.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Support/BuryPointer.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TimeProfiler.h"
#include "llvm/Support/Timer.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Transforms/Coroutines.h"
#include "llvm/Transforms/Coroutines/CoroCleanup.h"
#include "llvm/Transforms/Coroutines/CoroEarly.h"
#include "llvm/Transforms/Coroutines/CoroElide.h"
#include "llvm/Transforms/Coroutines/CoroSplit.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/IPO/LowerTypeTests.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/IPO/ThinLTOBitcodeWriter.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Instrumentation.h"
#include "llvm/Transforms/Instrumentation/AddressSanitizer.h"
#include "llvm/Transforms/Instrumentation/BoundsChecking.h"
#include "llvm/Transforms/Instrumentation/GCOVProfiler.h"
#include "llvm/Transforms/Instrumentation/HWAddressSanitizer.h"
#include "llvm/Transforms/Instrumentation/InstrProfiling.h"
#include "llvm/Transforms/Instrumentation/MemorySanitizer.h"
#include "llvm/Transforms/Instrumentation/SanitizerCoverage.h"
#include "llvm/Transforms/Instrumentation/ThreadSanitizer.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/CanonicalizeAliases.h"
#include "llvm/Transforms/Utils/EntryExitInstrumenter.h"
#include "llvm/Transforms/Utils/NameAnonGlobals.h"
#include "llvm/Transforms/Utils/SymbolRewriter.h"
#include "llvm/Transforms/Utils/UniqueInternalLinkageNames.h"
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

        void CreatePasses(llvm::legacy::PassManager &MPM, llvm::legacy::FunctionPassManager &FPM);

        /// Generates the TargetMachine.
        /// Leaves TM unchanged if it is unable to create the target machine.
        /// Some of our fly tests specify triples which are not built
        /// into fly. This is okay because these tests check the generated
        /// IR, and they require DataLayout which depends on the triple.
        /// In this case, we allow this method to fail and not report an error.
        /// When MustCreateTM is used, we print an error if we are unable to load
        /// the requested target.
        void CreateTargetMachine(bool MustCreateTM);

        /// Add passes necessary to emit assembly or LLVM IR.
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

        void EmitAssemblyWithNewPassManager(BackendActionKind Action,
                                            std::unique_ptr<raw_pwrite_stream> OS);
    };

// We need this wrapper to access LangOpts and CGOpts from extension functions
// that we add to the PassManagerBuilder.
class PassManagerBuilderWrapper : public llvm::PassManagerBuilder {
    public:
        PassManagerBuilderWrapper(const llvm::Triple &TargetTriple, const CodeGenOptions &CGOpts)
                : PassManagerBuilder(), TargetTriple(TargetTriple), CGOpts(CGOpts) {}
        const llvm::Triple &getTargetTriple() const { return TargetTriple; }
        const CodeGenOptions &getCGOpts() const { return CGOpts; }

    private:
        const llvm::Triple &TargetTriple;
        const CodeGenOptions &CGOpts;
    };
}

static void addAddDiscriminatorsPass(const llvm::PassManagerBuilder &Builder,
                                     llvm::legacy::PassManagerBase &PM) {
    PM.add(llvm::createAddDiscriminatorsPass());
}

static void addBoundsCheckingPass(const llvm::PassManagerBuilder &Builder,
                                  llvm::legacy::PassManagerBase &PM) {
    PM.add(llvm::createBoundsCheckingLegacyPass());
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

static void addSanitizerCoveragePass(const llvm::PassManagerBuilder &Builder,
                                     llvm::legacy::PassManagerBase &PM) {
    const PassManagerBuilderWrapper &BuilderWrapper =
            static_cast<const PassManagerBuilderWrapper &>(Builder);
    const CodeGenOptions &CGOpts = BuilderWrapper.getCGOpts();
    auto Opts = getSancovOptsFromCGOpts(CGOpts);
    PM.add(createModuleSanitizerCoverageLegacyPassPass(
            Opts, CGOpts.SanitizeCoverageAllowlistFiles,
            CGOpts.SanitizeCoverageBlocklistFiles));
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
        case llvm::Triple::UnknownObjectFormat:
            break;
    }
    return false;
}

static void addAddressSanitizerPasses(const llvm::PassManagerBuilder &Builder,
                                      llvm::legacy::PassManagerBase &PM) {
    const PassManagerBuilderWrapper &BuilderWrapper =
            static_cast<const PassManagerBuilderWrapper&>(Builder);
    const llvm::Triple &T = BuilderWrapper.getTargetTriple();
    const CodeGenOptions &CGOpts = BuilderWrapper.getCGOpts();
    bool Recover = CGOpts.SanitizeRecover.has(SanitizerKind::Address);
    bool UseAfterScope = CGOpts.SanitizeAddressUseAfterScope;
    bool UseOdrIndicator = CGOpts.SanitizeAddressUseOdrIndicator;
    bool UseGlobalsGC = asanUseGlobalsGC(T, CGOpts);
    PM.add(llvm::createAddressSanitizerFunctionPass(/*CompileKernel*/ false, Recover,
                                                                UseAfterScope));
    PM.add(llvm::createModuleAddressSanitizerLegacyPassPass(
            /*CompileKernel*/ false, Recover, UseGlobalsGC, UseOdrIndicator));
}

static void addKernelAddressSanitizerPasses(const llvm::PassManagerBuilder &Builder,
                                            llvm::legacy::PassManagerBase &PM) {
    PM.add(llvm::createAddressSanitizerFunctionPass(
            /*CompileKernel*/ true, /*Recover*/ true, /*UseAfterScope*/ false));
    PM.add(llvm::createModuleAddressSanitizerLegacyPassPass(
            /*CompileKernel*/ true, /*Recover*/ true, /*UseGlobalsGC*/ true,
            /*UseOdrIndicator*/ false));
}

static void addHWAddressSanitizerPasses(const llvm::PassManagerBuilder &Builder,
                                        llvm::legacy::PassManagerBase &PM) {
    const PassManagerBuilderWrapper &BuilderWrapper =
            static_cast<const PassManagerBuilderWrapper &>(Builder);
    const CodeGenOptions &CGOpts = BuilderWrapper.getCGOpts();
    bool Recover = CGOpts.SanitizeRecover.has(SanitizerKind::HWAddress);
    PM.add(
            llvm::createHWAddressSanitizerLegacyPassPass(/*CompileKernel*/ false, Recover));
}

static void addKernelHWAddressSanitizerPasses(const llvm::PassManagerBuilder &Builder,
                                              llvm::legacy::PassManagerBase &PM) {
    PM.add(llvm::createHWAddressSanitizerLegacyPassPass(
            /*CompileKernel*/ true, /*Recover*/ true));
}

static void addGeneralOptsForMemorySanitizer(const llvm::PassManagerBuilder &Builder,
                                             llvm::legacy::PassManagerBase &PM,
                                             bool CompileKernel) {
    const PassManagerBuilderWrapper &BuilderWrapper =
            static_cast<const PassManagerBuilderWrapper&>(Builder);
    const CodeGenOptions &CGOpts = BuilderWrapper.getCGOpts();
    int TrackOrigins = CGOpts.SanitizeMemoryTrackOrigins;
    bool Recover = CGOpts.SanitizeRecover.has(SanitizerKind::Memory);
    PM.add(llvm::createMemorySanitizerLegacyPassPass(
            llvm::MemorySanitizerOptions{TrackOrigins, Recover, CompileKernel}));

    // MemorySanitizer inserts complex instrumentation that mostly follows
    // the logic of the original code, but operates on "shadow" values.
    // It can benefit from re-running some general purpose optimization passes.
    if (Builder.OptLevel > 0) {
        PM.add(llvm::createEarlyCSEPass());
        PM.add(llvm::createReassociatePass());
        PM.add(llvm::createLICMPass());
        PM.add(llvm::createGVNPass());
        PM.add(llvm::createInstructionCombiningPass());
        PM.add(llvm::createDeadStoreEliminationPass());
    }
}

static void addMemorySanitizerPass(const llvm::PassManagerBuilder &Builder,
                                   llvm::legacy::PassManagerBase &PM) {
    addGeneralOptsForMemorySanitizer(Builder, PM, /*CompileKernel*/ false);
}

static void addKernelMemorySanitizerPass(const llvm::PassManagerBuilder &Builder,
                                         llvm::legacy::PassManagerBase &PM) {
    addGeneralOptsForMemorySanitizer(Builder, PM, /*CompileKernel*/ true);
}

static void addThreadSanitizerPass(const llvm::PassManagerBuilder &Builder,
                                   llvm::legacy::PassManagerBase &PM) {
    PM.add(llvm::createThreadSanitizerLegacyPassPass());
}

static void addDataFlowSanitizerPass(const llvm::PassManagerBuilder &Builder,
                                     llvm::legacy::PassManagerBase &PM) {
    const PassManagerBuilderWrapper &BuilderWrapper =
            static_cast<const PassManagerBuilderWrapper&>(Builder);
    // TODO SanitizerBlacklistFiles
    //PM.add(createDataFlowSanitizerPass(LangOpts.SanitizerBlacklistFiles));
}

static llvm::TargetLibraryInfoImpl *createTLII(llvm::Triple &TargetTriple, const CodeGenOptions &CodeGenOpts) {
    llvm::TargetLibraryInfoImpl *TLII = new llvm::TargetLibraryInfoImpl(TargetTriple);

    switch (CodeGenOpts.getVecLib()) {
        case CodeGenOptions::Accelerate:
            TLII->addVectorizableFunctionsFromVecLib(llvm::TargetLibraryInfoImpl::Accelerate);
            break;
        case CodeGenOptions::MASSV:
            TLII->addVectorizableFunctionsFromVecLib(llvm::TargetLibraryInfoImpl::MASSV);
            break;
        case CodeGenOptions::SVML:
            TLII->addVectorizableFunctionsFromVecLib(llvm::TargetLibraryInfoImpl::SVML);
            break;
        default:
            break;
    }
    return TLII;
}

static void addSymbolRewriterPass(const CodeGenOptions &Opts,
                                  llvm::legacy::PassManager *MPM) {
    llvm::SymbolRewriter::RewriteDescriptorList DL;

    llvm::SymbolRewriter::RewriteMapParser MapParser;
    for (const auto &MapFile : Opts.RewriteMapFiles)
        MapParser.parse(MapFile, &DL);

    MPM->add(llvm::createRewriteSymbolsPass(DL));
}

static llvm::CodeGenOpt::Level getCGOptLevel(const CodeGenOptions &CodeGenOpts) {
    switch (CodeGenOpts.OptimizationLevel) {
        default:
            llvm_unreachable("Invalid optimization level!");
        case 0:
            return llvm::CodeGenOpt::None;
        case 1:
            return llvm::CodeGenOpt::Less;
        case 2:
            return llvm::CodeGenOpt::Default; // O2/Os/Oz
        case 3:
            return llvm::CodeGenOpt::Aggressive;
    }
}

static Optional<llvm::CodeModel::Model> getCodeModel(const CodeGenOptions &CodeGenOpts) {
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
        return None;
    return static_cast<llvm::CodeModel::Model>(CodeModel);
}

static llvm::CodeGenFileType getCodeGenFileType(BackendActionKind Action) {
    if (Action == Backend_EmitObj)
        return llvm::CGFT_ObjectFile;
    else if (Action == Backend_EmitNothing)
        return llvm::CGFT_Null;
    else {
        assert(Action == Backend_EmitAssembly && "Invalid action!");
        return llvm::CGFT_AssemblyFile;
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
    Options.CompressDebugSections = CodeGenOpts.getCompressDebugSections();
    Options.RelaxELFRelocations = CodeGenOpts.RelaxELFRelocations;

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
    Options.StackAlignmentOverride = CodeGenOpts.StackAlignment;

    Options.BBSections =
            llvm::StringSwitch<llvm::BasicBlockSection>(CodeGenOpts.BBSections)
                    .Case("all", llvm::BasicBlockSection::All)
                    .Case("labels", llvm::BasicBlockSection::Labels)
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
    Options.ExplicitEmulatedTLS = CodeGenOpts.ExplicitEmulatedTLS;
    Options.DebuggerTuning = CodeGenOpts.getDebuggerTuning();
    Options.EmitStackSizeSection = CodeGenOpts.StackSizeSection;
    Options.EmitAddrsig = CodeGenOpts.Addrsig;
    Options.ForceDwarfFrameSection = CodeGenOpts.ForceDwarfFrameSection;
    Options.EmitCallSiteInfo = CodeGenOpts.EmitCallSiteInfo;
    Options.XRayOmitFunctionIndex = CodeGenOpts.XRayOmitFunctionIndex;

    Options.MCOptions.SplitDwarfFile = CodeGenOpts.SplitDwarfFile;
    Options.MCOptions.MCRelaxAll = CodeGenOpts.RelaxAll;
    Options.MCOptions.MCSaveTempLabels = CodeGenOpts.SaveTempLabels;
    Options.MCOptions.MCUseDwarfDirectory = !CodeGenOpts.NoDwarfDirectoryAsm;
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
    Options.MCOptions.Argv0 = CodeGenOpts.Argv0;
    Options.MCOptions.CommandLineArgs = CodeGenOpts.CommandLineArgs;
}
static Optional<llvm::GCOVOptions> getGCOVOptions(const CodeGenOptions &CodeGenOpts) {
    if (CodeGenOpts.DisableGCov)
        return None;
    if (!CodeGenOpts.EmitGcovArcs && !CodeGenOpts.EmitGcovNotes)
        return None;
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

static Optional<llvm::InstrProfOptions> getInstrProfOptions(const CodeGenOptions &CodeGenOpts) {
    if (!CodeGenOpts.hasProfileClangInstr())
        return None;
    llvm::InstrProfOptions Options;
    Options.NoRedZone = CodeGenOpts.DisableRedZone;
    Options.InstrProfileOutput = CodeGenOpts.InstrProfileOutput;

    // TODO: Surface the option to emit atomic profile counter increments at
    // the driver level.
    Options.Atomic = false;
    return Options;
}

void EmitAssemblyHelper::CreatePasses(llvm::legacy::PassManager &MPM,
                                      llvm::legacy::FunctionPassManager &FPM) {
    // Handle disabling of all LLVM passes, where we want to preserve the
    // internal module before any optimization.
    if (CodeGenOpts.DisableLLVMPasses)
        return;

    // Figure out TargetLibraryInfo.  This needs to be added to MPM and FPM
    // manually (and not via PMBuilder), since some passes (eg. InstrProfiling)
    // are inserted before PMBuilder ones - they'd get the default-constructed
    // TLI with an unknown target otherwise.
    llvm::Triple TargetTriple(TheModule->getTargetTriple());
    std::unique_ptr<llvm::TargetLibraryInfoImpl> TLII(
            createTLII(TargetTriple, CodeGenOpts));

    // If we reached here with a non-empty index file name, then the index file
    // was empty and we are not performing ThinLTO backend compilation (used in
    // testing in a distributed build environment). Drop any the type test
    // assume sequences inserted for whole program vtables so that codegen doesn't
    // complain.
    if (!CodeGenOpts.ThinLTOIndexFile.empty())
        MPM.add(llvm::createLowerTypeTestsPass(/*ExportSummary=*/nullptr,
                /*ImportSummary=*/nullptr,
                /*DropTypeTests=*/true));

    PassManagerBuilderWrapper PMBuilder(TargetTriple, CodeGenOpts);

    // At O0 and O1 we only run the always inliner which is more efficient. At
    // higher optimization levels we run the normal inliner.
    if (CodeGenOpts.OptimizationLevel <= 1) {
        bool InsertLifetimeIntrinsics = ((CodeGenOpts.OptimizationLevel != 0 && !CodeGenOpts.DisableLifetimeMarkers));
        PMBuilder.Inliner = llvm::createAlwaysInlinerLegacyPass(InsertLifetimeIntrinsics);
    } else {
        // We do not want to inline hot callsites for SamplePGO module-summary build
        // because profile annotation will happen again in ThinLTO backend, and we
        // want the IR of the hot path to match the profile.
        PMBuilder.Inliner = llvm::createFunctionInliningPass(
                CodeGenOpts.OptimizationLevel, CodeGenOpts.OptimizeSize,
                (!CodeGenOpts.SampleProfileFile.empty() &&
                 CodeGenOpts.PrepareForThinLTO));
    }

    PMBuilder.OptLevel = CodeGenOpts.OptimizationLevel;
    PMBuilder.SizeLevel = CodeGenOpts.OptimizeSize;
    PMBuilder.SLPVectorize = CodeGenOpts.VectorizeSLP;
    PMBuilder.LoopVectorize = CodeGenOpts.VectorizeLoop;
    // Only enable CGProfilePass when using integrated assembler, since
    // non-integrated assemblers don't recognize .cgprofile section.
    PMBuilder.CallGraphProfile = !CodeGenOpts.DisableIntegratedAS;

    PMBuilder.DisableUnrollLoops = !CodeGenOpts.UnrollLoops;
    // Loop interleaving in the loop vectorizer has historically been set to be
    // enabled when loop unrolling is enabled.
    PMBuilder.LoopsInterleaved = CodeGenOpts.UnrollLoops;
    PMBuilder.MergeFunctions = CodeGenOpts.MergeFunctions;
    PMBuilder.PrepareForThinLTO = CodeGenOpts.PrepareForThinLTO;
    PMBuilder.PrepareForLTO = CodeGenOpts.PrepareForLTO;
    PMBuilder.RerollLoops = CodeGenOpts.RerollLoops;

    MPM.add(new llvm::TargetLibraryInfoWrapperPass(*TLII));

    if (TM)
        TM->adjustPassManager(PMBuilder);

    if (CodeGenOpts.DebugInfoForProfiling ||
        !CodeGenOpts.SampleProfileFile.empty())
        PMBuilder.addExtension(llvm::PassManagerBuilder::EP_EarlyAsPossible,
                               addAddDiscriminatorsPass);

    // TODO Add the main ARC optimization passes?
//    PMBuilder.addExtension(PassManagerBuilder::EP_EarlyAsPossible, addObjCARCExpandPass);
//    PMBuilder.addExtension(PassManagerBuilder::EP_ModuleOptimizerEarly, addObjCARCAPElimPass);
//    PMBuilder.addExtension(PassManagerBuilder::EP_ScalarOptimizerLate, addObjCARCOptPass);


    // TODO Coroutines?
//    addCoroutinePassesToExtensionPoints(PMBuilder);

    // TODO bounds checking?
//    PMBuilder.addExtension(PassManagerBuilder::EP_ScalarOptimizerLate, addBoundsCheckingPass);
//    PMBuilder.addExtension(PassManagerBuilder::EP_EnabledOnOptLevel0, addBoundsCheckingPass);

    if (CodeGenOpts.SanitizeCoverageType ||
        CodeGenOpts.SanitizeCoverageIndirectCalls ||
        CodeGenOpts.SanitizeCoverageTraceCmp) {
        PMBuilder.addExtension(llvm::PassManagerBuilder::EP_OptimizerLast,
                               addSanitizerCoveragePass);
        PMBuilder.addExtension(llvm::PassManagerBuilder::EP_EnabledOnOptLevel0,
                               addSanitizerCoveragePass);
    }

    // TODO sanitize address?
//    PMBuilder.addExtension(PassManagerBuilder::EP_OptimizerLast, addAddressSanitizerPasses);
//    PMBuilder.addExtension(PassManagerBuilder::EP_EnabledOnOptLevel0, addAddressSanitizerPasses);

    // TODO sanitize kernel?
//    PMBuilder.addExtension(PassManagerBuilder::EP_OptimizerLast, addKernelAddressSanitizerPasses);
//    PMBuilder.addExtension(PassManagerBuilder::EP_EnabledOnOptLevel0, addKernelAddressSanitizerPasses);

    // TODO sanitize HWAddress?
//    PMBuilder.addExtension(PassManagerBuilder::EP_OptimizerLast, addHWAddressSanitizerPasses);
//    PMBuilder.addExtension(PassManagerBuilder::EP_EnabledOnOptLevel0, addHWAddressSanitizerPasses);

    // TODO sanitize KernelHWAddress?
//    PMBuilder.addExtension(PassManagerBuilder::EP_OptimizerLast, addKernelHWAddressSanitizerPasses);
//    PMBuilder.addExtension(PassManagerBuilder::EP_EnabledOnOptLevel0, addKernelHWAddressSanitizerPasses);

    // TODO sanitize Memory?
//    PMBuilder.addExtension(PassManagerBuilder::EP_OptimizerLast, addMemorySanitizerPass);
//    PMBuilder.addExtension(PassManagerBuilder::EP_EnabledOnOptLevel0, addMemorySanitizerPass);

    // TODO sanitize KernelMemory?
//    PMBuilder.addExtension(PassManagerBuilder::EP_OptimizerLast, addKernelMemorySanitizerPass);
//    PMBuilder.addExtension(PassManagerBuilder::EP_EnabledOnOptLevel0, addKernelMemorySanitizerPass);

    // TODO sanitize Thread?
//    PMBuilder.addExtension(PassManagerBuilder::EP_OptimizerLast, addThreadSanitizerPass);
//    PMBuilder.addExtension(PassManagerBuilder::EP_EnabledOnOptLevel0, addThreadSanitizerPass);

    // TODO sanitize DataFlow?
//    PMBuilder.addExtension(PassManagerBuilder::EP_OptimizerLast, addDataFlowSanitizerPass);
//    PMBuilder.addExtension(PassManagerBuilder::EP_EnabledOnOptLevel0, addDataFlowSanitizerPass);

    // Set up the per-function pass manager.
    FPM.add(new llvm::TargetLibraryInfoWrapperPass(*TLII));
    if (CodeGenOpts.VerifyModule)
        FPM.add(llvm::createVerifierPass());

    // Set up the per-module pass manager.
    if (!CodeGenOpts.RewriteMapFiles.empty())
        addSymbolRewriterPass(CodeGenOpts, &MPM);

    // Add UniqueInternalLinkageNames Pass which renames internal linkage symbols
    // with unique names.
    if (CodeGenOpts.UniqueInternalLinkageNames) {
        MPM.add(llvm::createUniqueInternalLinkageNamesPass());
    }

    if (Optional<llvm::GCOVOptions> Options = getGCOVOptions(CodeGenOpts)) {
        MPM.add(llvm::createGCOVProfilerPass(*Options));
        if (CodeGenOpts.getDebugInfo() == codegenoptions::NoDebugInfo)
            MPM.add(llvm::createStripSymbolsPass(true));
    }

    if (Optional<llvm::InstrProfOptions> Options =
            getInstrProfOptions(CodeGenOpts))
        MPM.add(llvm::createInstrProfilingLegacyPass(*Options, false));

    bool hasIRInstr = false;
    if (CodeGenOpts.hasProfileIRInstr()) {
        PMBuilder.EnablePGOInstrGen = true;
        hasIRInstr = true;
    }
    if (CodeGenOpts.hasProfileCSIRInstr()) {
        assert(!CodeGenOpts.hasProfileCSIRUse() &&
               "Cannot have both CSProfileUse pass and CSProfileGen pass at the "
               "same time");
        assert(!hasIRInstr &&
               "Cannot have both ProfileGen pass and CSProfileGen pass at the "
               "same time");
        PMBuilder.EnablePGOCSInstrGen = true;
        hasIRInstr = true;
    }
    if (hasIRInstr) {
        if (!CodeGenOpts.InstrProfileOutput.empty())
            PMBuilder.PGOInstrGen = CodeGenOpts.InstrProfileOutput;
        else
            PMBuilder.PGOInstrGen = std::string(DefaultProfileGenName);
    }
    if (CodeGenOpts.hasProfileIRUse()) {
        PMBuilder.PGOInstrUse = CodeGenOpts.ProfileInstrumentUsePath;
        PMBuilder.EnablePGOCSInstrUse = CodeGenOpts.hasProfileCSIRUse();
    }

    if (!CodeGenOpts.SampleProfileFile.empty())
        PMBuilder.PGOSampleUse = CodeGenOpts.SampleProfileFile;

    PMBuilder.populateFunctionPassManager(FPM);
    PMBuilder.populateModulePassManager(MPM);
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

    Optional<llvm::CodeModel::Model> CM = getCodeModel(CodeGenOpts);
    std::string FeaturesStr =
            llvm::join(TargetOpts.Features.begin(), TargetOpts.Features.end(), ",");
    llvm::Reloc::Model RM = CodeGenOpts.RelocationModel;
    llvm::CodeGenOpt::Level OptLevel = getCGOptLevel(CodeGenOpts);

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

void EmitAssemblyHelper::EmitAssembly(BackendActionKind Action,
                                      std::unique_ptr<raw_pwrite_stream> OS) {
    llvm::TimeRegion Region(FrontendTimesIsEnabled ? &CodeGenerationTime : nullptr);

    setCommandLineOpts(CodeGenOpts);

    bool UsesCodeGen = (Action != Backend_EmitNothing &&
                        Action != Backend_EmitBC &&
                        Action != Backend_EmitLL);
    CreateTargetMachine(UsesCodeGen);

    if (UsesCodeGen && !TM)
        return;
    if (TM)
        TheModule->setDataLayout(TM->createDataLayout());

    llvm::legacy::PassManager PerModulePasses;
    PerModulePasses.add(
            createTargetTransformInfoWrapperPass(getTargetIRAnalysis()));

    llvm::legacy::FunctionPassManager PerFunctionPasses(TheModule);
    PerFunctionPasses.add(
            createTargetTransformInfoWrapperPass(getTargetIRAnalysis()));

    CreatePasses(PerModulePasses, PerFunctionPasses);

    llvm::legacy::PassManager CodeGenPasses;
    CodeGenPasses.add(createTargetTransformInfoWrapperPass(getTargetIRAnalysis()));

    std::unique_ptr<llvm::ToolOutputFile> ThinLinkOS, DwoOS;

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
                PerModulePasses.add(createWriteThinLTOBitcodePass(
                        *OS, ThinLinkOS ? &ThinLinkOS->os() : nullptr));
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

                PerModulePasses.add(createBitcodeWriterPass(
                        *OS, CodeGenOpts.EmitLLVMUseLists, EmitLTOSummary));
            }
            break;

        case Backend_EmitLL:
            PerModulePasses.add(
                    createPrintModulePass(*OS, "", CodeGenOpts.EmitLLVMUseLists));
            break;

        default:
            if (!CodeGenOpts.SplitDwarfOutput.empty()) {
                DwoOS = openOutputFile(CodeGenOpts.SplitDwarfOutput);
                if (!DwoOS)
                    return;
            }
            if (!AddEmitPasses(CodeGenPasses, Action, *OS,
                               DwoOS ? &DwoOS->os() : nullptr))
                return;
    }

    // Before executing passes, print the final values of the LLVM options.
    llvm::cl::PrintOptionValues();

    // Run passes. For now we do all passes at once, but eventually we
    // would like to have the option of streaming code generation.

    {
        llvm::PrettyStackTraceString CrashInfo("Per-function optimization");
        llvm::TimeTraceScope TimeScope("PerFunctionPasses");

        PerFunctionPasses.doInitialization();
        for (llvm::Function &F : *TheModule)
            if (!F.isDeclaration())
                PerFunctionPasses.run(F);
        PerFunctionPasses.doFinalization();
    }

    {
        llvm::PrettyStackTraceString CrashInfo("Per-module optimization passes");
        llvm::TimeTraceScope TimeScope("PerModulePasses");
        PerModulePasses.run(*TheModule);
    }

    {
        llvm::PrettyStackTraceString CrashInfo("Code generation");
        llvm::TimeTraceScope TimeScope("CodeGenPasses");
        CodeGenPasses.run(*TheModule);
    }

    if (ThinLinkOS)
        ThinLinkOS->keep();
    if (DwoOS)
        DwoOS->keep();
}

static llvm::PassBuilder::OptimizationLevel mapToLevel(const CodeGenOptions &Opts) {
    switch (Opts.OptimizationLevel) {
        default:
            llvm_unreachable("Invalid optimization level!");

        case 1:
            return llvm::PassBuilder::OptimizationLevel::O1;

        case 2:
            switch (Opts.OptimizeSize) {
                default:
                    llvm_unreachable("Invalid optimization level for size!");

                case 0:
                    return llvm::PassBuilder::OptimizationLevel::O2;

                case 1:
                    return llvm::PassBuilder::OptimizationLevel::Os;

                case 2:
                    return llvm::PassBuilder::OptimizationLevel::Oz;
            }

        case 3:
            return llvm::PassBuilder::OptimizationLevel::O3;
    }
}

static void addCoroutinePassesAtO0(llvm::ModulePassManager &MPM, const CodeGenOptions &CodeGenOpts) {
    // TODO
//    if (!LangOpts.Coroutines)
//        return;

    MPM.addPass(llvm::createModuleToFunctionPassAdaptor(llvm::CoroEarlyPass()));

    llvm::CGSCCPassManager CGPM(CodeGenOpts.DebugPassManager);
    CGPM.addPass(llvm::CoroSplitPass());
    CGPM.addPass(llvm::createCGSCCToFunctionPassAdaptor(llvm::CoroElidePass()));
    MPM.addPass(createModuleToPostOrderCGSCCPassAdaptor(std::move(CGPM)));

    MPM.addPass(llvm::createModuleToFunctionPassAdaptor(llvm::CoroCleanupPass()));
}

static void addSanitizersAtO0(llvm::ModulePassManager &MPM,
                              const llvm::Triple &TargetTriple,
                              const CodeGenOptions &CodeGenOpts) {
    if (CodeGenOpts.SanitizeCoverageType ||
        CodeGenOpts.SanitizeCoverageIndirectCalls ||
        CodeGenOpts.SanitizeCoverageTraceCmp) {
        auto SancovOpts = getSancovOptsFromCGOpts(CodeGenOpts);
        MPM.addPass(llvm::ModuleSanitizerCoveragePass(
                SancovOpts, CodeGenOpts.SanitizeCoverageAllowlistFiles,
                CodeGenOpts.SanitizeCoverageBlocklistFiles));
    }

    auto ASanPass = [&](SanitizerMask Mask, bool CompileKernel) {
        MPM.addPass(llvm::RequireAnalysisPass<llvm::ASanGlobalsMetadataAnalysis, llvm::Module>());
        bool Recover = CodeGenOpts.SanitizeRecover.has(Mask);
        MPM.addPass(llvm::createModuleToFunctionPassAdaptor(llvm::AddressSanitizerPass(
                CompileKernel, Recover, CodeGenOpts.SanitizeAddressUseAfterScope)));
        bool ModuleUseAfterScope = asanUseGlobalsGC(TargetTriple, CodeGenOpts);
        MPM.addPass(
                llvm::ModuleAddressSanitizerPass(CompileKernel, Recover, ModuleUseAfterScope,
                                           CodeGenOpts.SanitizeAddressUseOdrIndicator));
    };

    // TODO
//    if (LangOpts.Sanitize.has(SanitizerKind::Address)) {
//        ASanPass(SanitizerKind::Address, /*CompileKernel=*/false);
//    }
//
//    if (LangOpts.Sanitize.has(SanitizerKind::KernelAddress)) {
//        ASanPass(SanitizerKind::KernelAddress, /*CompileKernel=*/true);
//    }
//
//    if (LangOpts.Sanitize.has(SanitizerKind::Memory)) {
//        bool Recover = CodeGenOpts.SanitizeRecover.has(SanitizerKind::Memory);
//        int TrackOrigins = CodeGenOpts.SanitizeMemoryTrackOrigins;
//        MPM.addPass(MemorySanitizerPass({TrackOrigins, Recover, false}));
//        MPM.addPass(createModuleToFunctionPassAdaptor(
//                MemorySanitizerPass({TrackOrigins, Recover, false})));
//    }
//
//    if (LangOpts.Sanitize.has(SanitizerKind::KernelMemory)) {
//        MPM.addPass(createModuleToFunctionPassAdaptor(
//                MemorySanitizerPass({0, false, /*Kernel=*/true})));
//    }
//
//    if (LangOpts.Sanitize.has(SanitizerKind::Thread)) {
//        MPM.addPass(ThreadSanitizerPass());
//        MPM.addPass(createModuleToFunctionPassAdaptor(ThreadSanitizerPass()));
//    }
}

/// A clean version of `EmitAssembly` that uses the new pass manager.
///
/// Not all features are currently supported in this system, but where
/// necessary it falls back to the legacy pass manager to at least provide
/// basic functionality.
///
/// This API is planned to have its functionality finished and then to replace
/// `EmitAssembly` at some point in the future when the default switches.
void EmitAssemblyHelper::EmitAssemblyWithNewPassManager(
        BackendActionKind Action, std::unique_ptr<raw_pwrite_stream> OS) {
    llvm::TimeRegion Region(&CodeGenerationTime);
    setCommandLineOpts(CodeGenOpts);

    bool RequiresCodeGen = (Action != Backend_EmitNothing &&
                            Action != Backend_EmitBC &&
                            Action != Backend_EmitLL);
    CreateTargetMachine(RequiresCodeGen);

    if (RequiresCodeGen && !TM)
        return;
    if (TM)
        TheModule->setDataLayout(TM->createDataLayout());

    Optional<llvm::PGOOptions> PGOOpt;

    if (CodeGenOpts.hasProfileIRInstr())
        // -fprofile-generate.
        PGOOpt = llvm::PGOOptions(CodeGenOpts.InstrProfileOutput.empty()
                            ? std::string(DefaultProfileGenName)
                            : CodeGenOpts.InstrProfileOutput,
                            "", "", llvm::PGOOptions::IRInstr, llvm::PGOOptions::NoCSAction,
                            CodeGenOpts.DebugInfoForProfiling);
    else if (CodeGenOpts.hasProfileIRUse()) {
        // -fprofile-use.
        auto CSAction = CodeGenOpts.hasProfileCSIRUse() ? llvm::PGOOptions::CSIRUse
                                                        : llvm::PGOOptions::NoCSAction;
        PGOOpt = llvm::PGOOptions(CodeGenOpts.ProfileInstrumentUsePath, "",
                            CodeGenOpts.ProfileRemappingFile, llvm::PGOOptions::IRUse,
                            CSAction, CodeGenOpts.DebugInfoForProfiling);
    } else if (!CodeGenOpts.SampleProfileFile.empty())
        // -fprofile-sample-use
        PGOOpt =
                llvm::PGOOptions(CodeGenOpts.SampleProfileFile, "",
                           CodeGenOpts.ProfileRemappingFile, llvm::PGOOptions::SampleUse,
                           llvm::PGOOptions::NoCSAction, CodeGenOpts.DebugInfoForProfiling);
    else if (CodeGenOpts.DebugInfoForProfiling)
        // -fdebug-info-for-profiling
        PGOOpt = llvm::PGOOptions("", "", "", llvm::PGOOptions::NoAction,
                            llvm::PGOOptions::NoCSAction, true);

    // Check to see if we want to generate a CS profile.
    if (CodeGenOpts.hasProfileCSIRInstr()) {
        assert(!CodeGenOpts.hasProfileCSIRUse() &&
               "Cannot have both CSProfileUse pass and CSProfileGen pass at "
               "the same time");
        if (PGOOpt.hasValue()) {
            assert(PGOOpt->Action != llvm::PGOOptions::IRInstr &&
                   PGOOpt->Action != llvm::PGOOptions::SampleUse &&
                   "Cannot run CSProfileGen pass with ProfileGen or SampleUse "
                   " pass");
            PGOOpt->CSProfileGenFile = CodeGenOpts.InstrProfileOutput.empty()
                                       ? std::string(DefaultProfileGenName)
                                       : CodeGenOpts.InstrProfileOutput;
            PGOOpt->CSAction = llvm::PGOOptions::CSIRInstr;
        } else
            PGOOpt = llvm::PGOOptions("",
                                CodeGenOpts.InstrProfileOutput.empty()
                                ? std::string(DefaultProfileGenName)
                                : CodeGenOpts.InstrProfileOutput,
                                "", llvm::PGOOptions::NoAction, llvm::PGOOptions::CSIRInstr,
                                CodeGenOpts.DebugInfoForProfiling);
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

    // TODO
//    PTO.Coroutines = LangOpts.Coroutines;

    llvm::PassInstrumentationCallbacks PIC;
    llvm::StandardInstrumentations SI;
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

    llvm::LoopAnalysisManager LAM(CodeGenOpts.DebugPassManager);
    llvm::FunctionAnalysisManager FAM(CodeGenOpts.DebugPassManager);
    llvm::CGSCCAnalysisManager CGAM(CodeGenOpts.DebugPassManager);
    llvm::ModuleAnalysisManager MAM(CodeGenOpts.DebugPassManager);

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

    llvm::ModulePassManager MPM(CodeGenOpts.DebugPassManager);

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
                        /*DropTypeTests=*/true));
            if (Optional<llvm::GCOVOptions> Options = getGCOVOptions(CodeGenOpts))
                MPM.addPass(llvm::GCOVProfilerPass(*Options));
            if (Optional<llvm::InstrProfOptions> Options = getInstrProfOptions(CodeGenOpts))
                MPM.addPass(llvm::InstrProfiling(*Options, false));

            // Build a minimal pipeline based on the semantics required by Clang,
            // which is just that always inlining occurs. Further, disable generating
            // lifetime intrinsics to avoid enabling further optimizations during
            // code generation.
            // However, we need to insert lifetime intrinsics to avoid invalid access
            // caused by multithreaded coroutines.
            // TODO
//            MPM.addPass(AlwaysInlinerPass(/*InsertLifetimeIntrinsics=*/LangOpts.Coroutines));
            MPM.addPass(llvm::AlwaysInlinerPass(/*InsertLifetimeIntrinsics=*/false));

            // At -O0, we can still do PGO. Add all the requested passes for
            // instrumentation PGO, if requested.
            if (PGOOpt && (PGOOpt->Action == llvm::PGOOptions::IRInstr ||
                           PGOOpt->Action == llvm::PGOOptions::IRUse))
                PB.addPGOInstrPassesForO0(
                        MPM, CodeGenOpts.DebugPassManager,
                        /* RunProfileGen */ (PGOOpt->Action == llvm::PGOOptions::IRInstr),
                        /* IsCS */ false, PGOOpt->ProfileFile,
                        PGOOpt->ProfileRemappingFile);

            // At -O0 we directly run necessary sanitizer passes.
            // TODO
//            if (LangOpts.Sanitize.has(SanitizerKind::LocalBounds))
//                MPM.addPass(createModuleToFunctionPassAdaptor(BoundsCheckingPass()));

            // Add UniqueInternalLinkageNames Pass which renames internal linkage
            // symbols with unique names.
            if (CodeGenOpts.UniqueInternalLinkageNames) {
                MPM.addPass(llvm::UniqueInternalLinkageNamesPass());
            }

            // Lastly, add semantically necessary passes for LTO.
            if (IsLTO || IsThinLTO) {
                MPM.addPass(llvm::CanonicalizeAliasesPass());
                MPM.addPass(llvm::NameAnonGlobalPass());
            }
        } else {
            // Map our optimization levels into one of the distinct levels used to
            // configure the pipeline.
            llvm::PassBuilder::OptimizationLevel Level = mapToLevel(CodeGenOpts);

            // If we reached here with a non-empty index file name, then the index
            // file was empty and we are not performing ThinLTO backend compilation
            // (used in testing in a distributed build environment). Drop any the type
            // test assume sequences inserted for whole program vtables so that
            // codegen doesn't complain.
            if (!CodeGenOpts.ThinLTOIndexFile.empty())
                PB.registerPipelineStartEPCallback([](llvm::ModulePassManager &MPM) {
                    MPM.addPass(llvm::LowerTypeTestsPass(/*ExportSummary=*/nullptr,
                            /*ImportSummary=*/nullptr,
                            /*DropTypeTests=*/true));
                });

            PB.registerPipelineStartEPCallback([](llvm::ModulePassManager &MPM) {
                MPM.addPass(llvm::createModuleToFunctionPassAdaptor(
                        llvm::EntryExitInstrumenterPass(/*PostInlining=*/false)));
            });

            // Register callbacks to schedule sanitizer passes at the appropriate part of
            // the pipeline.
            // TODO
//            if (LangOpts.Sanitize.has(SanitizerKind::LocalBounds))
//                PB.registerScalarOptimizerLateEPCallback(
//                        [](FunctionPassManager &FPM, PassBuilder::OptimizationLevel Level) {
//                            FPM.addPass(BoundsCheckingPass());
//                        });

            if (CodeGenOpts.SanitizeCoverageType ||
                CodeGenOpts.SanitizeCoverageIndirectCalls ||
                CodeGenOpts.SanitizeCoverageTraceCmp) {
                PB.registerOptimizerLastEPCallback(
                        [this](llvm::ModulePassManager &MPM,
                               llvm::PassBuilder::OptimizationLevel Level) {
                            auto SancovOpts = getSancovOptsFromCGOpts(CodeGenOpts);
                            MPM.addPass(llvm::ModuleSanitizerCoveragePass(
                                    SancovOpts, CodeGenOpts.SanitizeCoverageAllowlistFiles,
                                    CodeGenOpts.SanitizeCoverageBlocklistFiles));
                        });
            }

            // TODO
//            if (LangOpts.Sanitize.has(SanitizerKind::Memory)) {
//                int TrackOrigins = CodeGenOpts.SanitizeMemoryTrackOrigins;
//                bool Recover = CodeGenOpts.SanitizeRecover.has(SanitizerKind::Memory);
//                PB.registerOptimizerLastEPCallback(
//                        [TrackOrigins, Recover](ModulePassManager &MPM,
//                                                PassBuilder::OptimizationLevel Level) {
//                            MPM.addPass(MemorySanitizerPass({TrackOrigins, Recover, false}));
//                            MPM.addPass(createModuleToFunctionPassAdaptor(
//                                    MemorySanitizerPass({TrackOrigins, Recover, false})));
//                        });
//            }
            // TODO
//            if (LangOpts.Sanitize.has(SanitizerKind::Thread)) {
//                PB.registerOptimizerLastEPCallback(
//                        [](ModulePassManager &MPM, PassBuilder::OptimizationLevel Level) {
//                            MPM.addPass(ThreadSanitizerPass());
//                            MPM.addPass(
//                                    createModuleToFunctionPassAdaptor(ThreadSanitizerPass()));
//                        });
//            }
            // TODO
//            if (LangOpts.Sanitize.has(SanitizerKind::Address)) {
//                bool Recover = CodeGenOpts.SanitizeRecover.has(SanitizerKind::Address);
//                bool UseAfterScope = CodeGenOpts.SanitizeAddressUseAfterScope;
//                bool ModuleUseAfterScope = asanUseGlobalsGC(TargetTriple, CodeGenOpts);
//                bool UseOdrIndicator = CodeGenOpts.SanitizeAddressUseOdrIndicator;
//                PB.registerOptimizerLastEPCallback(
//                        [Recover, UseAfterScope, ModuleUseAfterScope, UseOdrIndicator](
//                                ModulePassManager &MPM, PassBuilder::OptimizationLevel Level) {
//                            MPM.addPass(
//                                    RequireAnalysisPass<ASanGlobalsMetadataAnalysis, Module>());
//                            MPM.addPass(ModuleAddressSanitizerPass(
//                                    /*CompileKernel=*/false, Recover, ModuleUseAfterScope,
//                                                      UseOdrIndicator));
//                            MPM.addPass(
//                                    createModuleToFunctionPassAdaptor(AddressSanitizerPass(
//                                            /*CompileKernel=*/false, Recover, UseAfterScope)));
//                        });
//            }
            if (Optional<llvm::GCOVOptions> Options = getGCOVOptions(CodeGenOpts))
                PB.registerPipelineStartEPCallback([Options](llvm::ModulePassManager &MPM) {
                    MPM.addPass(llvm::GCOVProfilerPass(*Options));
                });
            // TODO
//            if (Optional<InstrProfOptions> Options =
//                    getInstrProfOptions(CodeGenOpts, LangOpts))
//                PB.registerPipelineStartEPCallback([Options](ModulePassManager &MPM) {
//                    MPM.addPass(InstrProfiling(*Options, false));
//                });

            // Add UniqueInternalLinkageNames Pass which renames internal linkage
            // symbols with unique names.
            if (CodeGenOpts.UniqueInternalLinkageNames) {
                MPM.addPass(llvm::UniqueInternalLinkageNamesPass());
            }

            if (IsThinLTO) {
                MPM = PB.buildThinLTOPreLinkDefaultPipeline(
                        Level, CodeGenOpts.DebugPassManager);
                MPM.addPass(llvm::CanonicalizeAliasesPass());
                MPM.addPass(llvm::NameAnonGlobalPass());
            } else if (IsLTO) {
                MPM = PB.buildLTOPreLinkDefaultPipeline(Level,
                                                        CodeGenOpts.DebugPassManager);
                MPM.addPass(llvm::CanonicalizeAliasesPass());
                MPM.addPass(llvm::NameAnonGlobalPass());
            } else {
                MPM = PB.buildPerModuleDefaultPipeline(Level,
                                                       CodeGenOpts.DebugPassManager);
            }
        }

        // TODO
//        if (LangOpts.Sanitize.has(SanitizerKind::HWAddress)) {
//            bool Recover = CodeGenOpts.SanitizeRecover.has(SanitizerKind::HWAddress);
//            MPM.addPass(HWAddressSanitizerPass(
//                    /*CompileKernel=*/false, Recover));
//        }
//        if (LangOpts.Sanitize.has(SanitizerKind::KernelHWAddress)) {
//            MPM.addPass(HWAddressSanitizerPass(
//                    /*CompileKernel=*/true, /*Recover=*/true));
//        }
//
//        if (CodeGenOpts.OptimizationLevel == 0) {
//            addCoroutinePassesAtO0(MPM, LangOpts, CodeGenOpts);
//            addSanitizersAtO0(MPM, TargetTriple, LangOpts, CodeGenOpts);
//        }
    }

    // FIXME: We still use the legacy pass manager to do code generation. We
    // create that pass manager here and use it as needed below.
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

    // We can simply import the values mentioned in the combined index, since
    // we should only invoke this using the individual indexes written out
    // via a WriteIndexesThinBackend.
    llvm::FunctionImporter::ImportMapTy ImportList;
    for (auto &GlobalList : *CombinedIndex) {
        // Ignore entries for undefined references.
        if (GlobalList.second.SummaryList.empty())
            continue;

        auto GUID = GlobalList.first;
        for (auto &Summary : GlobalList.second.SummaryList) {
            // Skip the summaries for the importing module. These are included to
            // e.g. record required linkage changes.
            if (Summary->modulePath() == M->getModuleIdentifier())
                continue;
            // Add an entry to provoke importing by thinBackend.
            ImportList[Summary->modulePath()].insert(GUID);
        }
    }

    std::vector<std::unique_ptr<llvm::MemoryBuffer>> OwnedImports;
    llvm::MapVector<llvm::StringRef, llvm::BitcodeModule> ModuleMap;

    for (auto &I : ImportList) {
        llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> MBOrErr =
                llvm::MemoryBuffer::getFile(I.first());
        if (!MBOrErr) {
            llvm::errs() << "Error loading imported file '" << I.first()
                   << "': " << MBOrErr.getError().message() << "\n";
            return;
        }

        Expected<llvm::BitcodeModule> BMOrErr = FindThinLTOModule(**MBOrErr);
        if (!BMOrErr) {
            handleAllErrors(BMOrErr.takeError(), [&](llvm::ErrorInfoBase &EIB) {
                llvm::errs() << "Error loading imported file '" << I.first()
                       << "': " << EIB.message() << '\n';
            });
            return;
        }
        ModuleMap.insert({I.first(), *BMOrErr});

        OwnedImports.push_back(std::move(*MBOrErr));
    }
    auto AddStream = [&](size_t Task) {
        return std::make_unique<llvm::lto::NativeObjectStream>(std::move(OS));
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
    Conf.UseNewPM = CGOpts.ExperimentalNewPassManager;
    Conf.DebugPassManager = CGOpts.DebugPassManager;
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
                WriteBitcodeToFile(*M, *OS, CGOpts.EmitLLVMUseLists);
                return false;
            };
            break;
        default:
            Conf.CGFileType = getCodeGenFileType(Action);
            break;
    }
    if (llvm::Error E = thinBackend(
            Conf, -1, AddStream, *M, *CombinedIndex, ImportList,
            ModuleToDefinedGVSummaries[M->getModuleIdentifier()], ModuleMap)) {
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

    if (CGOpts.ExperimentalNewPassManager)
        AsmHelper.EmitAssemblyWithNewPassManager(Action, std::move(OS));
    else
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
    llvm::EmbedBitcodeInModule(
            *M, Buf, CGOpts.getEmbedBitcode() != CodeGenOptions::Embed_Marker,
            CGOpts.getEmbedBitcode() != CodeGenOptions::Embed_Bitcode,
            &CGOpts.CmdArgs);
}