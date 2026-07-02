//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Frontend/Frontend.cpp - main compiler frontend
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Frontend/Frontend.h"

#include "AST/ASTAttribute.h"
#include "AST/ASTBuilder.h"
#include "AST/ASTClass.h"
#include "AST/ASTFunction.h"
#include "AST/ASTImport.h"
#include "AST/ASTMethod.h"
#include "AST/ASTModifier.h"
#include "AST/ASTModule.h"
#include "AST/ASTName.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTParam.h"
#include "AST/ASTType.h"
#include "AST/ASTVar.h"
#include "Frontend/InputFile.h"
#include "Basic/Archiver.h"
#include "Basic/Debug.h"
#include "Basic/Stack.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "Parser/Parser.h"
#include "Sema/SemaBuiltin.h"
#include "Sema/SemaContext.h"
#include "Sema/SemaModule.h"

#include <fstream>
#include <iostream>
#include <map>
#include <llvm/ADT/Statistic.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Timer.h>

using namespace fly;

// Join a relative output path under OutDir; absolute paths and empty OutDir pass through.
static std::string underOutDir(llvm::StringRef OutDir, llvm::StringRef Path) {
    if (OutDir.empty() || Path.empty() || llvm::sys::path::is_absolute(Path))
        return Path.str();
    llvm::SmallString<256> P(OutDir);
    llvm::sys::path::append(P, Path);
    return std::string(P.str());
}

// ─── Header generation helpers ───────────────────────────────────────────────

static std::string typeStr(const ASTType *T) {
    if (!T) return "";
    if (T->getTypeKind() == ASTTypeKind::TYPE_BUILTIN) {
        const auto *BT = static_cast<const ASTBuiltinType *>(T);
        switch (BT->getBuiltinKind()) {
            case ASTBuiltinTypeKind::TYPE_BOOL:   return "bool";
            case ASTBuiltinTypeKind::TYPE_BYTE:   return "byte";
            case ASTBuiltinTypeKind::TYPE_SHORT:  return "short";
            case ASTBuiltinTypeKind::TYPE_INT:    return "int";
            case ASTBuiltinTypeKind::TYPE_LONG:   return "long";
            case ASTBuiltinTypeKind::TYPE_USHORT: return "ushort";
            case ASTBuiltinTypeKind::TYPE_UINT:   return "uint";
            case ASTBuiltinTypeKind::TYPE_ULONG:  return "ulong";
            case ASTBuiltinTypeKind::TYPE_POINTER: return "pointer";
            case ASTBuiltinTypeKind::TYPE_FLOAT:  return "float";
            case ASTBuiltinTypeKind::TYPE_DOUBLE: return "double";
            case ASTBuiltinTypeKind::TYPE_STRING: return "string";
            default: return "";
        }
    }
    if (T->getTypeKind() == ASTTypeKind::TYPE_NAMED) {
        const auto *NT = static_cast<const ASTNamedType *>(T);
        std::string name;
        for (const auto *N : NT->getNames()) {
            if (!name.empty()) name += ".";
            name += N->getName().str();
        }
        // Preserve generic type arguments so a header'd `List<string>` parameter
        // stays instantiable (without this the `<string>` is dropped and the
        // generic class can't be resolved at the call site).
        const auto &Args = NT->getTypeArgs();
        if (!Args.empty()) {
            name += "<";
            bool first = true;
            for (const auto *A : Args) {
                if (!first) name += ", ";
                name += typeStr(A);
                first = false;
            }
            name += ">";
        }
        return name;
    }
    return "";
}

static std::string paramStr(const ASTParam *P) {
    std::string out;
    for (auto *Mod : P->getModifiers())
        if (Mod->getModifierKind() == ASTModifierKind::MOD_CONSTANT)
            out += "const ";
    out += typeStr(P->getType()) + " " + P->getName().str();
    return out;
}

static std::string funcSignatureStr(const ASTFunction *F) {
    std::string sig = "public ";
    const std::string ret = typeStr(F->getReturnType());
    if (!ret.empty()) sig += ret + " ";
    sig += F->getName().str() + "(";
    bool first = true;
    for (const auto *P : F->getParams()) {
        if (!first) sig += ", ";
        sig += paramStr(P);
        first = false;
    }
    sig += ")";
    return sig;
}

// Writes a .fly.h declaration file for the public API of M.
// Returns the path of the generated file, or empty string on failure.
// Modules that contain generic classes are skipped: they cannot be represented
// as declaration-only stubs because monomorphization requires the full method
// bodies.  Callers must ensure the .fly source is available in the lib dir so
// LoadLibHeaders falls through to load it directly.
static std::string GenerateHeader(ASTModule *M, DiagnosticsEngine &Diags,
                                   llvm::StringRef OutDir = "") {
    // When OutDir is set (--lib / --shared), write the header flat into OutDir
    // so it lands alongside the archive rather than next to the source file.
    auto makeHeaderPath = [&](llvm::StringRef stem) -> std::string {
        if (OutDir.empty())
            return stem.str() + ".fly.h";
        return (OutDir + "/" + llvm::sys::path::filename(stem) + ".fly.h").str();
    };

    // For modules with generic classes the header IS the full source:
    // ParseHeader skips import statements and detects <T> to force
    // SkipBodies=false, so monomorphization gets the complete method bodies.
    bool hasGenericClasses = false;
    for (const auto *Node : M->getNodes()) {
        if (Node->getKind() == ASTKind::AST_CLASS) {
            const auto *C = static_cast<const ASTClass *>(Node);
            if (!C->getTypeParams().empty()) { hasGenericClasses = true; break; }
        }
    }
    if (hasGenericClasses) {
        std::string SourcePath = M->getName() + ".fly";
        std::string HeaderPath = makeHeaderPath(M->getName());
        auto MBOrErr = llvm::MemoryBuffer::getFile(SourcePath);
        if (!MBOrErr) return "";
        std::error_code EC;
        llvm::raw_fd_ostream OS(HeaderPath, EC, llvm::sys::fs::OF_Text);
        if (EC) return "";
        OS << MBOrErr.get()->getBuffer();
        return HeaderPath;
    }

    // Derive header path: <source>.fly → <source>.fly.h (or OutDir/<stem>.fly.h)
    std::string HeaderPath = makeHeaderPath(M->getName());

    std::error_code EC;
    llvm::raw_fd_ostream OS(HeaderPath, EC, llvm::sys::fs::OF_Text);
    if (EC) {
        llvm::errs() << "error: cannot write header '" << HeaderPath << "': " << EC.message() << "\n";
        return "";
    }

    // Namespace declaration
    if (ASTNameSpace *NS = M->getNameSpace()) {
        OS << "namespace ";
        bool first = true;
        for (const auto *N : NS->getNames()) {
            if (!first) OS << ".";
            OS << N->getName();
            first = false;
        }
        OS << "\n\n";
    }

    // Public struct declarations (must precede interface/class/function declarations)
    for (const auto *Node : M->getNodes()) {
        if (Node->getKind() != ASTKind::AST_CLASS) continue;
        const auto *C = static_cast<const ASTClass *>(Node);
        if (C->getClassKind() != ASTClassKind::STRUCT) continue;
        bool isPublic = false;
        for (auto *Mod : C->getModifiers())
            if (Mod->getModifierKind() == ASTModifierKind::MOD_PUBLIC)
                isPublic = true;
        if (!isPublic) continue;
        OS << "public struct " << C->getName() << " {\n";
        for (const auto *Field : C->getNodes()) {
            if (Field->getKind() != ASTKind::AST_VAR) continue;
            const auto *V = static_cast<const ASTVar *>(Field);
            OS << "    " << typeStr(V->getType()) << " " << V->getName() << "\n";
        }
        OS << "}\n\n";
    }

    // Public interface declarations (must precede class declarations that implement them)
    for (const auto *Node : M->getNodes()) {
        if (Node->getKind() != ASTKind::AST_CLASS) continue;
        const auto *C = static_cast<const ASTClass *>(Node);
        if (C->getClassKind() != ASTClassKind::INTERFACE) continue;
        bool isPublic = false;
        for (auto *Mod : C->getModifiers())
            if (Mod->getModifierKind() == ASTModifierKind::MOD_PUBLIC)
                isPublic = true;
        if (!isPublic) continue;
        OS << "public interface " << C->getName() << " {\n";
        for (const auto *Child : C->getNodes()) {
            // Methods in classes/interfaces use AST_FUNCTION kind (ASTMethod extends ASTFunction)
            if (Child->getKind() != ASTKind::AST_FUNCTION) continue;
            const auto *Meth = static_cast<const ASTMethod *>(Child);
            // Emit return type so callers know the calling convention (hidden out-pointer).
            const std::string ret = typeStr(Meth->getReturnType());
            if (!ret.empty()) OS << "    " << ret << " ";
            else OS << "    void ";
            OS << Meth->getName().str() << "(";
            bool first = true;
            for (const auto *P : Meth->getParams()) {
                if (!first) OS << ", ";
                OS << paramStr(P);
                first = false;
            }
            OS << ")\n";
        }
        OS << "}\n\n";
    }

    // Public class declarations (concrete classes; must follow interfaces they implement)
    for (const auto *Node : M->getNodes()) {
        if (Node->getKind() != ASTKind::AST_CLASS) continue;
        const auto *C = static_cast<const ASTClass *>(Node);
        if (C->getClassKind() != ASTClassKind::CLASS) continue;
        bool isPublic = false;
        for (auto *Mod : C->getModifiers())
            if (Mod->getModifierKind() == ASTModifierKind::MOD_PUBLIC)
                isPublic = true;
        if (!isPublic) continue;
        OS << "public class " << C->getName();
        bool firstBase = true;
        for (const auto *Base : C->getBases()) {
            if (firstBase) { OS << " : "; firstBase = false; }
            else OS << ", ";
            OS << typeStr(Base);
        }
        OS << " {\n";
        for (const auto *Child : C->getNodes()) {
            if (Child->getKind() == ASTKind::AST_VAR) {
                const auto *V = static_cast<const ASTVar *>(Child);
                // Emit all fields (including private) so the object layout in the
                // header matches the compiled implementation in fly_std_lib.a.
                // Skip fields whose type cannot be expressed as a simple name (they
                // would require imports not present in the header).
                const std::string fieldTy = typeStr(V->getType());
                if (fieldTy.empty()) continue;
                OS << "    " << fieldTy << " " << V->getName() << "\n";
            } else if (Child->getKind() == ASTKind::AST_FUNCTION) {
                // Methods use AST_FUNCTION kind (ASTMethod extends ASTFunction)
                const auto *Meth = static_cast<const ASTMethod *>(Child);
                bool methPublic = false;
                for (auto *Mod : Meth->getModifiers())
                    if (Mod->getModifierKind() == ASTModifierKind::MOD_PUBLIC)
                        methPublic = true;
                if (!methPublic) continue;
                bool isCtor = (Meth->getName() == C->getName());
                OS << "    public ";
                if (!isCtor) {
                    // Emit return type (void methods need explicit 'void').
                    const std::string ret = typeStr(Meth->getReturnType());
                    if (!ret.empty()) OS << ret << " ";
                    else OS << "void ";
                }
                OS << Meth->getName().str() << "(";
                bool first = true;
                for (const auto *P : Meth->getParams()) {
                    if (!first) OS << ", ";
                    OS << paramStr(P);
                    first = false;
                }
                OS << ") {}\n";
            }
        }
        OS << "}\n\n";
    }

    // Public function signatures
    for (const auto *Node : M->getNodes()) {
        if (Node->getKind() != ASTKind::AST_FUNCTION) continue;
        const auto *F = static_cast<const ASTFunction *>(Node);
        bool isPublic = false;
        for (auto *Mod : F->getModifiers())
            if (Mod->getModifierKind() == ASTModifierKind::MOD_PUBLIC)
                isPublic = true;
        if (!isPublic) continue;
        OS << funcSignatureStr(F) << "\n";
    }

    return HeaderPath;
}

Frontend::Frontend(CompilerInstance &CI) : CI(CI), Diags(CI.getDiagnostics()) {

}

Frontend::~Frontend() {

}

bool Frontend::Execute() {
    assert(!CI.getFrontendOptions().ShowHelp && "Client must handle '-help'!");
    assert(!CI.getFrontendOptions().ShowVersion && "Client must handle '-version'!");
    FLY_DEBUG_SCOPE("Frontend", "Execute");

    raw_ostream &OS = llvm::errs();

    // Create Timers and show after compilation
    if (CI.getFrontendOptions().ShowTimers)
        CreateFrontendTimer();

    if (CI.getFrontendOptions().ShowStats)
        llvm::EnableStatistics(false);

    // Check if Input Files not empty
    if (CI.getFrontendOptions().getInputFiles().empty()) {
        Diags.Report(SourceLocation(), diag::note_fe_no_input_process);
        return false;
    }

    // Parse input files
	// Init the Sema Builder
	ASTBuilder *Builder = new ASTBuilder(Diags);

    // Load stdlib headers (.fly.h), then external package dirs (.fly.h).
    if (!CI.getFrontendOptions().StdLibDir.empty())
        LoadLibHeaders(*Builder, CI.getFrontendOptions().StdLibDir, /*preferDotFlyH=*/true);
    for (const auto &Dir : CI.getFrontendOptions().LibDirs)
        LoadLibHeaders(*Builder, Dir, /*preferDotFlyH=*/true);

    const auto &AllInputs = CI.getFrontendOptions().getInputFiles();

    // Parse the entry file (first input) before auto-detection / dep-resolution.
    {
        Diags.getClient()->BeginSourceFile();
        ParseFile(*Builder, AllInputs[0]);
        Diags.getClient()->EndSourceFile();
    }

    // Auto-detect output type from the entry file's AST (single-file build with no
    // --lib/--shared). May set CreateLibrary/TestMode and auto-name the output.
    if (CI.getFrontendOptions().AutoDetectOutput && !ASTModules.empty())
        AutoDetectOutputType(ASTModules.back());

    // --out-dir: create the directory and resolve the final artifact (-o or
    // auto-named) under it, so ToolChain and the header-dir derivation pick it up.
    // Done after auto-detect so the auto-named output is redirected too.
    {
        const std::string &OutDir = CI.getFrontendOptions().OutDir;
        if (!OutDir.empty()) {
            llvm::sys::fs::create_directories(OutDir);
            std::string Out = CI.getFrontendOptions().getOutputFile();
            if (!Out.empty())
                CI.getFrontendOptions().setOutputFile(
                    underOutDir(OutDir, Out), CI.getFrontendOptions().isOutputLib());
        }
    }

    // Resolve import-based source dependencies ONLY when --src-dir is given. This is
    // a deliberate opt-in: dependency pulling is never implicit, so explicit/per-file
    // invocations (e.g. flyp's single-source per-target builds) compile exactly the
    // sources they were handed, and the user keeps full control over composition.
    if (!CI.getFrontendOptions().SrcDirs.empty())
        ResolveSourceDeps(*Builder);

    // Parse remaining explicit input files (multi-file CLI mode, unchanged).
    for (size_t i = 1; i < AllInputs.size(); ++i) {
        Diags.getClient()->BeginSourceFile();
        ParseFile(*Builder, AllInputs[i]);
        Diags.getClient()->EndSourceFile();
    }

	// Parse files, create AST, build Semantics checker
	SemaContext *S = new SemaContext(Diags);

    Diags.getClient()->BeginSourceFile();

    // Resolve AST references; store on the member so getSemaModules() works
    // after Execute() returns (used by the LSP server and other tools).
	SemaModules = S->Resolve(ASTModules, CI.getCodeGenOptions().TestMode);

    if (!SemaModules.empty()) {

        // Create LLVM Context (must outlive all modules)
        llvm::LLVMContext LLVMCtx;

        // Generate Backend Code
        // Parsers and InputFiles must remain alive through CodeGen: AST StringRefs
        // (class/function names, type names, etc.) point into the Lexer's IdentifierTable.
        CodeGen CG(Diags, LLVMCtx, CI.getCodeGenOptions(), CI.getTargetOptions(),
                   CI.getFrontendOptions().BackendAction,
                   CI.getFrontendOptions().ShowTimers);
        CG.setSourceManager(CI.getSourceManager());
        SmallVector<SemaModule *, 8> CompilableModules;
        for (auto *SM : SemaModules)
            if (!SM->getAST().isHeader())
                CompilableModules.push_back(SM);
        // A single explicit -o output means a single linked artifact (lib/shared/exe):
        // lower all input files into one module so cross-file references resolve.
        // Without -o, each file is emitted to its own .ll/.bc/.s/.o.
        bool SingleModule = !CI.getFrontendOptions().getOutputFile().empty();
        llvm::SmallVector<llvm::Module *, 8> Modules = CG.GenerateModules(CompilableModules, SingleModule);

        // Emit code base on BackendActionKind
        const std::string &OutDir = CI.getFrontendOptions().OutDir;
        for (auto M : Modules) {
            // M->getName() is the source filename (e.g. "main.fly"); getOutputFileName
            // turns it into "main.fly.o" / "main.fly.ll" etc. Redirect under --out-dir
            // when set, and pass the resolved path to Emit so it lands there.
            std::string OutFile = underOutDir(OutDir, CG.getOutputFileName(M->getName()));
            CG.Emit(M, OutFile);
            if (!OutFile.empty())
                OutputFiles.push_back(OutFile);
        }

    	// Delete generated LLVM modules (ownership transferred from CodeGenModule here)
    	for (auto *M : Modules)
    		delete M;
    	Modules.clear();

        // Generate .fly.h headers for each non-header module when --lib/--header is set.
        if (CI.getFrontendOptions().CreateHeader) {
            std::string HdrDir;
            if (CI.getFrontendOptions().CreateLibrary ||
                CI.getFrontendOptions().CreateSharedLib) {
                HdrDir = llvm::sys::path::parent_path(
                             CI.getFrontendOptions().getOutputFile()).str();
                // Output archive sits in the CWD (e.g. auto-named "Parser.a" or
                // "-o foo.a"): keep the headers alongside it rather than scattering
                // them next to each source file.
                if (HdrDir.empty())
                    HdrDir = ".";
            }
            // --out-dir wins: it also covers the bare --header (non-lib) case, which
            // would otherwise write headers next to each source file.
            if (!OutDir.empty())
                HdrDir = OutDir;
            for (auto *SM : CompilableModules) {
                std::string HPath = GenerateHeader(&SM->getAST(), Diags, HdrDir);
                if (!HPath.empty())
                    OutputFiles.push_back(HPath);
            }
        }
    }

    // Reset CodeGen pointers on SemaBuiltin singletons: these are shared across
    // compilations and their CodeGen pointers would become dangling after the
    // LLVMContext above goes out of scope.
    SemaBuiltin::resetCodeGen();

    // Release Parsers and InputFiles now that CodeGen is done accessing AST StringRefs.
    for (auto *P : Parsers) delete P;
    for (auto *I : InputFiles) delete I;
    Parsers.clear();
    InputFiles.clear();

    Diags.getClient()->EndSourceFile();

    // Finish client diagnostics
    Diags.getClient()->finish();

    // Show Errors Warnings and Notes
    if (CI.getDiagnostics().getDiagnosticOptions().ShowCarets) {
        // We can have multiple diagnostics sharing one diagnostic client.
        // Get the total number of warnings/errors from the client.
        unsigned NumWarnings = CI.getDiagnostics().getClient()->getNumWarnings();
        unsigned NumErrors = CI.getDiagnostics().getClient()->getNumErrors();

        if (NumWarnings)
            OS << NumWarnings << " warning" << (NumWarnings == 1 ? "" : "s");
        if (NumWarnings && NumErrors)
            OS << " and ";
        if (NumErrors)
            OS << NumErrors << " error" << (NumErrors == 1 ? "" : "s");
        if (NumWarnings || NumErrors) {
            OS << " generated";
            OS << ".\n";
        }
    }

    // Show Stats
    if (CI.getFrontendOptions().ShowStats) {
        CI.getFileManager().PrintStats();
        OS << '\n';
        llvm::PrintStatistics(OS);
    }
    StringRef StatsFile = CI.getFrontendOptions().StatsFile;
    if (!StatsFile.empty()) {
        std::error_code EC;
        auto StatS = std::make_unique<llvm::raw_fd_ostream>(
                StatsFile, EC, llvm::sys::fs::OF_Text);
        if (EC) {
            CI.getDiagnostics().Report(diag::warn_fe_unable_to_open_stats_file)
                    << StatsFile << EC.message();
        } else {
            llvm::PrintStatisticsJSON(*StatS);
        }
    }

    delete Builder;
    delete S;

    return !CI.getDiagnostics().getClient()->getNumErrors();
}

void Frontend::AutoDetectOutputType(ASTModule *M) {
    FLY_DEBUG_SCOPE("Frontend", "AutoDetectOutputType");
    bool hasMain = false, hasSuite = false;
    for (ASTNode *N : M->getNodes()) {
        if (N->getKind() == ASTKind::AST_FUNCTION) {
            auto *F = static_cast<ASTFunction *>(N);
            if (F->getName() == "main") hasMain = true;
        } else if (N->getKind() == ASTKind::AST_CLASS) {
            auto *C = static_cast<ASTClass *>(N);
            if (C->getClassKind() == ASTClassKind::SUITE) hasSuite = true;
        }
    }

    FrontendOptions &FO = CI.getFrontendOptions();
    bool testMode = CI.getCodeGenOptions().TestMode; // set by --test flag
    bool hasOutput = !FO.getOutputFile().empty();
    std::string stem = llvm::sys::path::stem(
        llvm::sys::path::filename(FO.getInputFiles()[0])).str();

    // Forced library (--lib/--shared): keep the library behaviour set by the Driver,
    // even when a main() is present. Only auto-name the output (ToolChain appends the
    // platform extension: .a/.lib for static, .so/.dylib/.dll for shared).
    if (FO.CreateLibrary || FO.CreateSharedLib) {
        if (!hasOutput) FO.setOutputFile(stem, /*isLib=*/true);
        FLY_DEBUG_MSG("Forced library output '" << stem << "'");
        return;
    }

    // Precedence: (suite | (main & --test)) → test exe; main → exe; else → lib.
    if (hasSuite || (hasMain && testMode)) {
        // Test executable: TestMode drives implicit main() generation when only a
        // suite is present; an explicit main() runs its test {} blocks.
        CI.getCodeGenOptions().TestMode = true;
        if (!hasOutput) FO.setOutputFile(stem);
        FLY_DEBUG_MSG("Auto-detected: test executable '" << stem << "'");
    } else if (hasMain) {
        // Plain executable.
        if (!hasOutput) FO.setOutputFile(stem);
        FLY_DEBUG_MSG("Auto-detected: executable '" << stem << "'");
    } else {
        // No main, no suite → static library + header.
        FO.CreateLibrary = true;
        FO.CreateHeader  = true;
        if (!hasOutput) FO.setOutputFile(stem, /*isLib=*/true);
        FLY_DEBUG_MSG("Auto-detected: static library '" << stem << "'");
    }
}

// Extract "namespace foo.bar" from the first non-comment, non-blank line of a file.
static std::string extractFileNamespace(const std::string &FilePath) {
    std::ifstream f(FilePath);
    std::string line;
    while (std::getline(f, line)) {
        size_t s = line.find_first_not_of(" \t");
        if (s == std::string::npos) continue;
        line = line.substr(s);
        if (line.empty() || line[0] == '/' || line[0] == '*') continue;
        if (line.size() >= 9 && line.substr(0, 9) == "namespace") {
            size_t ns = line.find_first_not_of(" \t", 9);
            if (ns != std::string::npos) {
                size_t end = line.find_first_of(" \t\r\n", ns);
                return line.substr(ns, end == std::string::npos ? end : end - ns);
            }
        }
        break;
    }
    return "";
}

// Reconstruct "fly.compiler" from an ASTImport's Names vector.
static std::string importNamespace(const ASTImport *Imp) {
    std::string ns;
    for (const auto *N : Imp->getNames()) {
        if (!ns.empty()) ns += ".";
        ns += N->getName().str();
    }
    return ns;
}

void Frontend::ResolveSourceDeps(ASTBuilder &Builder) {
    FLY_DEBUG_SCOPE("Frontend", "ResolveSourceDeps");
    FrontendOptions &FO = CI.getFrontendOptions();

    // Collect source search dirs: explicit --src-dir, or the entry file's parent dir.
    llvm::SmallVector<std::string, 4> Dirs = FO.SrcDirs;
    if (Dirs.empty()) {
        std::string parent = llvm::sys::path::parent_path(FO.getInputFiles()[0]).str();
        Dirs.push_back(parent.empty() ? "." : parent);
    }

    // Build namespace → [path] map from all .fly files in the source dirs.
    std::map<std::string, std::vector<std::string>> NsToFiles;
    llvm::StringSet<> KnownFiles; // files already parsed (entry + headers)
    for (const auto *M : ASTModules)
        KnownFiles.insert(llvm::sys::path::filename(M->getFile()->getFileName()));

    std::error_code EC;
    for (const auto &Dir : Dirs) {
        for (llvm::sys::fs::recursive_directory_iterator I(Dir, EC), E;
             I != E && !EC; I.increment(EC)) {
            const std::string &Path = I->path();
            llvm::StringRef PathRef(Path);
            if (!PathRef.ends_with(".fly") || PathRef.ends_with(".fly.h")) continue;
            if (KnownFiles.count(llvm::sys::path::filename(Path))) continue;
            std::string ns = extractFileNamespace(Path);
            if (!ns.empty()) NsToFiles[ns].push_back(Path);
        }
    }

    // BFS: for each parsed module, resolve its imports to source files and parse them.
    // ParseFile appends to ASTModules, so newly added modules are visited in turn.
    size_t i = 0;
    while (i < ASTModules.size()) {
        ASTModule *M = ASTModules[i++];
        for (ASTNode *N : M->getNodes()) {
            if (N->getKind() != ASTKind::AST_IMPORT) continue;
            auto *Imp = static_cast<ASTImport *>(N);
            std::string ns = importNamespace(Imp);
            auto it = NsToFiles.find(ns);
            // A plain/alias import is `Namespace.Symbol` (e.g. `import fly.compiler.ast.ASTNode`):
            // its trailing component is the imported class/enum/function name, NOT a namespace
            // component, so the joined path is not a declared namespace. Fall back to the parent
            // namespace (drop the last component) so the namespace's source files still get pulled
            // in. Wildcard imports already carry the bare namespace, so skip them here.
            if (it == NsToFiles.end() && !Imp->isWildcard()) {
                std::string parent = ns;
                while (it == NsToFiles.end()) {
                    auto dot = parent.rfind('.');
                    if (dot == std::string::npos) break;
                    parent = parent.substr(0, dot);
                    it = NsToFiles.find(parent);
                }
            }
            if (it == NsToFiles.end()) continue;
            for (const auto &Path : it->second) {
                llvm::StringRef Fname = llvm::sys::path::filename(Path);
                if (KnownFiles.count(Fname)) continue;
                KnownFiles.insert(Fname);
                FLY_DEBUG_MSG("Resolved import '" << ns << "' → " << Path);
                Diags.getClient()->BeginSourceFile();
                ParseFile(Builder, Path);
                Diags.getClient()->EndSourceFile();
            }
            // Done with this namespace; avoid re-processing it for other modules.
            NsToFiles.erase(it);
        }
    }
}

/**
 * ParseModule Input File.
 * @param CG
 * @return
 */
void Frontend::ParseFile(ASTBuilder &Builder, const std::string &FileName) {
    FLY_DEBUG_SCOPE_MSG("Frontend", "ParseFile", "Loading input file " + FileName);
    InputFile *Input = new InputFile(Diags, CI.getSourceManager(), FileName);
    if (Input->getExt() == FileExt::FLY) {
        if (Input->Load()) {
            // Create Parser and start to parse; keep alive until after Sema so
            // that StringRefs in AST nodes (pointing into Lexer's IdentifierTable)
            // remain valid throughout semantic analysis.
            Parser *P = new Parser(Input, CI.getSourceManager(), Diags, Builder);
            ASTModule *M = P->ParseModule();
        	ASTModules.push_back(M);
        	Parsers.push_back(P);
        	InputFiles.push_back(Input);
        	return;
        }
        delete Input;
    } else if (Input->getExt() == FileExt::LIB) {
        // Look for a <archive_stem>.fly.h file alongside the archive.
        // e.g. "mymath.a" → look for "mymath.fly.h" in the same directory.
        llvm::SmallString<256> HeaderPath(FileName);
        llvm::sys::path::replace_extension(HeaderPath, "fly.h");
        if (llvm::sys::fs::exists(HeaderPath)) {
            InputFile *InputHeader = new InputFile(Diags, CI.getSourceManager(), HeaderPath.str().str());
            if (InputHeader->Load()) {
                Parser *P = new Parser(InputHeader, CI.getSourceManager(), Diags, Builder);
                ASTModule *M = P->ParseHeader();
                if (M) ASTModules.push_back(M);
                Parsers.push_back(P);
                InputFiles.push_back(InputHeader);
            } else {
                delete InputHeader;
            }
        }
        delete Input;
    } else {
        CI.getDiagnostics().Report(diag::err_fe_input_file_ext) << FileName;
        delete Input;
    }
}

void Frontend::CreateFrontendTimer() {
    FLY_DEBUG_SCOPE("Frontend", "CreateFrontendTimer");
    FrontendTimerGroup.reset(
            new llvm::TimerGroup("frontend", "Clang front-end time report"));
    FrontendTimer.reset(
            new llvm::Timer("frontend", "Clang front-end timer",
                            *FrontendTimerGroup));
}

const SmallVector<std::string, 4> &Frontend::getOutputFiles() const {
    return OutputFiles;
}

void Frontend::LoadLibHeaders(ASTBuilder &Builder, const std::string &Dir,
                              bool preferDotFlyH) {
    // Build a set of filenames currently being compiled (e.g. "math.fly").
    // A source file whose basename is already an input is skipped: the in-memory
    // AST built from the full parse is authoritative.
    llvm::StringSet<> CompilingNow;
    for (const auto &F : CI.getFrontendOptions().getInputFiles())
        CompilingNow.insert(llvm::sys::path::filename(F));

    std::error_code EC;
    for (llvm::sys::fs::recursive_directory_iterator I(Dir, EC), E; I != E && !EC; I.increment(EC)) {
        const std::string &Path = I->path();
        llvm::StringRef Filename = llvm::sys::path::filename(Path);

        // Skip directories silently — the iterator descends into them automatically.
        if (llvm::sys::fs::is_directory(Path))
            continue;

        // Prefer .fly.h over .fly when both exist in the same directory.
        // A .fly.h is a generated declaration-only header with no imports:
        // loading it avoids transitive dependency issues when ParseHeader()
        // processes a .fly source that contains import statements.
        if (Filename.ends_with(".fly.h")) {
            // Load .fly.h only when preferDotFlyH is set (external package dirs).
            // In stdlib mode, .fly.h files are skipped — the .fly source is parsed.
            if (!preferDotFlyH)
                continue;
            // Skip if the corresponding .fly is currently being compiled as main input.
            std::string SourceName = Path.substr(0, Path.size() - 2); // strip ".h"
            llvm::StringRef SourceFilename = llvm::sys::path::filename(SourceName);
            if (CompilingNow.count(SourceFilename))
                continue;
            Diags.getClient()->BeginSourceFile();
            InputFile *Input = new InputFile(Diags, CI.getSourceManager(), Path);
            if (Input->Load()) {
                Parser *P = new Parser(Input, CI.getSourceManager(), Diags, Builder);
                ASTModule *M = P->ParseHeader();
                if (M) ASTModules.push_back(M);
                Parsers.push_back(P);
                InputFiles.push_back(Input);
            } else {
                delete Input;
            }
            Diags.getClient()->EndSourceFile();
            continue;
        }

        // Skip non-.fly files silently. Static/shared libraries (.a, .lib, .so,
        // .dylib) and object files legitimately share lib/ with .fly.h headers
        // (fly_std_lib.a, fly_runtime_lib.a live here by design).
        if (!Filename.ends_with(".fly"))
            continue;

        if (CompilingNow.count(Filename))
            continue;

        // When preferDotFlyH is set (external package dirs), skip .fly source when
        // a .fly.h companion exists — the header was already loaded above and is
        // preferred (imports-free, no transitive dependency issues).
        if (preferDotFlyH && llvm::sys::fs::exists(Path + ".h"))
            continue;

        InputFile *Input = new InputFile(Diags, CI.getSourceManager(), Path);
        if (Input->Load()) {
            Parser *P = new Parser(Input, CI.getSourceManager(), Diags, Builder);
            ASTModule *M = P->ParseHeader();
            if (M) ASTModules.push_back(M);
            Parsers.push_back(P);
            InputFiles.push_back(Input);
        } else {
            delete Input;
        }
    }
}


std::vector<StringRef> Frontend::ExtractFiles(const std::string &LibFileName) {
    Archiver Ar(Diags, LibFileName);
    std::vector<StringRef> List;
    if (Ar.ExtractLib(CI.getFileManager())) {
        return Ar.getExtractFiles();
    }
    return List;
}

bool Archiver::ExtractLib(FileManager &FileMgr) {
    FLY_DEBUG_SCOPE("Archiver", "ExtractLib");
    ErrorOr<std::unique_ptr<MemoryBuffer>> Buf =
            MemoryBuffer::getFile(ArchiveName, -1, false);
    std::error_code EC = Buf.getError();
    if (EC) {
        return fail("unable to open '" + ArchiveName + "': " + EC.message());
    }

    Error Err = Error::success();
    object::Archive Arch(Buf.get()->getMemBufferRef(), Err);
    if (isError(std::move(Err), "unable to load '" + ArchiveName + "'")) {
        return false;
    }
    return performReadOperation(Extract, &Arch);
}
