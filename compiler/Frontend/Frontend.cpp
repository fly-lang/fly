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
#include "AST/ASTEnum.h"
#include "AST/ASTEnumEntry.h"
#include "AST/ASTFunction.h"
#include "AST/ASTImport.h"
#include "AST/ASTMethod.h"
#include "AST/ASTModifier.h"
#include "AST/ASTModule.h"
#include "AST/ASTName.h"
#include "AST/ASTValue.h"
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
// DiscoverySkipsDir — directories the source walks NEVER descend into (B009):
// hidden dot-dirs (.git, .flyp, .claude, …) and `build` (stage outputs carry
// copied .fly sources — a stray one used to break root discovery with
// "multiple main()"). The self-host mirrors the same rule
// (fly.driver.discoverySkipsDir / Frontend.scanSrcDir).
static bool DiscoverySkipsDir(llvm::StringRef Name) {
    return Name.starts_with(".") || Name == "build";
}

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
    // Arrays had NO case here: an array-typed param, field or return type rendered as
    // the EMPTY STRING, so a generated .fly.h carried a nameless type and the header
    // could not be consumed back. Recursing on the element type covers `int[][]`.
    // Only a literal size is rendered — anything computed is not reproducible in a
    // header, and `T[]` is the honest spelling for it.
    if (T->getTypeKind() == ASTTypeKind::TYPE_ARRAY) {
        const auto *AT = static_cast<const ASTArrayType *>(T);
        std::string Elem = typeStr(AT->getElementType());
        if (Elem.empty()) return "";
        std::string Size;
        const ASTExpr *SizeExpr = AT->getSizeExpr();
        if (SizeExpr && SizeExpr->getExprKind() == ASTExprKind::EXPR_VALUE) {
            const auto *V = static_cast<const ASTValue *>(SizeExpr);
            if (V->getValueKind() == ASTValueKind::VAL_NUMBER)
                Size = static_cast<const ASTNumberValue *>(V)->getValue().str();
        }
        return Elem + "[" + Size + "]";
    }
    return "";
}

static std::string paramStr(const ASTParam *P) {
    std::string out;
    for (auto *Mod : P->getModifiers())
        if (Mod->getModifierKind() == ASTModifierKind::MOD_CONSTANT)
            out += "const ";
    out += typeStr(P->getType()) + " " + P->getName().str();
    // Preserve a defaulted param's literal value: without it a header consumer
    // must spell out every argument — the call-site default filling
    // (ArityPass::Defaulted) only sees defaults the declaration carries.
    // Only literal values exist here (the parser accepts nothing else after '=').
    const ASTExpr *E = P->getExpr();
    if (E && E->getExprKind() == ASTExprKind::EXPR_VALUE) {
        const auto *V = static_cast<const ASTValue *>(E);
        switch (V->getValueKind()) {
            case ASTValueKind::VAL_BOOL:
                out += static_cast<const ASTBoolValue *>(V)->getValue() ? " = true" : " = false";
                break;
            case ASTValueKind::VAL_NUMBER:
                out += " = " + static_cast<const ASTNumberValue *>(V)->getValue().str();
                break;
            case ASTValueKind::VAL_STRING:
                out += " = \"" + static_cast<const ASTStringValue *>(V)->getValue().str() + "\"";
                break;
            case ASTValueKind::VAL_NULL:
                out += " = null";
                break;
            default:
                break; // not representable in a header — omit the default
        }
    }
    return out;
}

static std::string funcSignatureStr(const ASTFunction *F) {
    std::string sig = "public ";
    const std::string ret = typeStr(F->getReturnType());
    // Always emit a return type: a void return has an empty/null type, but the
    // generated header must be explicit (the self-host parser rejects a function with
    // no return type). Mirrors the method-signature path.
    if (!ret.empty()) sig += ret + " ";
    else {
        // MULTI-RETURN (`A,B f(x)`): getReturnType() is null, but collapsing that to
        // `void` here DROPS the extra returns. A header consumer would then build the
        // call with too few arguments for the archive symbol's real ABI
        // (errPtr, params…, __out_0, …, __out_N) and crash in the backend.
        std::string multi;
        bool firstRT = true;
        for (const auto *RT : F->getReturnTypes()) {
            const std::string s = typeStr(RT);
            if (s.empty()) continue;
            if (!firstRT) multi += ", ";
            multi += s;
            firstRT = false;
        }
        if (!multi.empty()) sig += multi + " ";
        else sig += "void ";
    }
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
    // When OutDir is set (--lib / --lib-dyn), write the header flat into OutDir
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

    // Imports: a header's public API can reference types from OTHER namespaces (a
    // class implementing an interface from another namespace, a field/param of a
    // foreign type). Those short names only resolve if the module's imports are
    // present, so emit them here — ParseHeader consumes them (it used to skip
    // imports back when generated headers carried none).
    for (const auto *Node : M->getNodes()) {
        if (Node->getKind() != ASTKind::AST_IMPORT) continue;
        const auto *Imp = static_cast<const ASTImport *>(Node);
        OS << "import ";
        bool first = true;
        for (const auto *N : Imp->getNames()) {
            if (!first) OS << ".";
            OS << N->getName();
            first = false;
        }
        if (Imp->isWildcard()) OS << ".*";
        if (!Imp->getAlias().empty()) {
            OS << " as ";
            bool af = true;
            for (const auto *A : Imp->getAlias()) {
                if (!af) OS << ".";
                OS << A->getName();
                af = false;
            }
        }
        OS << "\n";
    }
    OS << "\n";

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

    // Public enum declarations — enum types (e.g. SemaKind) are referenced as
    // field/param types elsewhere, so their names must be declared and importable.
    for (const auto *Node : M->getNodes()) {
        if (Node->getKind() != ASTKind::AST_ENUM) continue;
        const auto *E = static_cast<const ASTEnum *>(Node);
        bool isPublic = false;
        for (auto *Mod : E->getModifiers())
            if (Mod->getModifierKind() == ASTModifierKind::MOD_PUBLIC)
                isPublic = true;
        if (!isPublic) continue;
        OS << "public enum " << E->getName() << " {\n";
        bool first = true;
        for (const auto *Child : E->getNodes()) {
            const auto *Entry = static_cast<const ASTEnumEntry *>(Child);
            if (!first) OS << ",\n";
            OS << "    " << Entry->getName();
            first = false;
        }
        OS << "\n}\n\n";
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

    // Directory mode (the CLI): no explicit inputs exist — discover them from the
    // source root. API users (tests, LSP) that pass explicit inputs skip this; an
    // empty input list without DiscoverInputs stays the historical error.
    if (CI.getFrontendOptions().getInputFiles().empty()) {
        if (!CI.getFrontendOptions().DiscoverInputs) {
            Diags.Report(SourceLocation(), diag::note_fe_no_input_process);
            return false;
        }
        if (!DiscoverInputs())
            return false;
    }

    // Parse input files
	// Init the Sema Builder
	ASTBuilder *Builder = new ASTBuilder(Diags);

    // Load stdlib headers (.fly.h), then external package dirs (.fly.h).
    // A -L dir's bare .fly sources are NOT loaded as declaration-only headers:
    // since the std left this repo (0.13.14 seed scenario) there is no archive to
    // define their symbols at link time, so ResolveSourceDeps treats the -L dirs
    // as source roots and pulls each imported namespace IN FULL instead. A .fly.h
    // in the -L dir still wins (its namespace links from the archive beside it).
    if (!CI.getFrontendOptions().StdLibDir.empty())
        LoadLibHeaders(*Builder, CI.getFrontendOptions().StdLibDir, /*preferDotFlyH=*/true);
    for (const auto &Dir : CI.getFrontendOptions().LibDirs)
        LoadLibHeaders(*Builder, Dir, /*preferDotFlyH=*/true, /*SkipBareSources=*/true);

    const auto &AllInputs = CI.getFrontendOptions().getInputFiles();

    // Parse every input file (the discovered entry set, or the API caller's list)
    // BEFORE dependency resolution, so the lazy source scan inside
    // ResolveSourceDeps already knows them all and never re-parses one as a dep.
    const size_t EntryIdx = ASTModules.size();
    for (const auto &In : AllInputs) {
        Diags.getClient()->BeginSourceFile();
        ParseFile(*Builder, In);
        Diags.getClient()->EndSourceFile();
    }

    // Auto-detect output type from the entry file's AST (linking build with no -o).
    // May set CreateLibrary/TestMode and auto-name the output.
    if (CI.getFrontendOptions().AutoDetectOutput && ASTModules.size() > EntryIdx)
        AutoDetectOutputType(ASTModules[EntryIdx]);

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

    // Resolve import-based source dependencies. A fly project is compiled from its
    // root directory: sources are discovered from the CURRENT directory by default,
    // and a single --src-dir overrides that root. The directory walk inside is lazy —
    // programs whose imports are all served by lib headers (std-only) never scan.
    ResolveSourceDeps(*Builder);

	// Parse files, create AST, build Semantics checker
	SemaContext *S = new SemaContext(Diags);

    Diags.getClient()->BeginSourceFile();

    // Resolve AST references; store on the member so getSemaModules() works
    // after Execute() returns (used by the LSP server and other tools).
    //
    // Only when the sources actually PARSED. Nothing but the parser has run yet,
    // so an error here means a malformed AST: resolving one walks half-built
    // identifier chains and null types until some Resolver/Registry consistency
    // check fires err_invalid_behavior, and the build ends on "internal compiler
    // error" instead of the syntax error the user needs to read (a plain typo,
    // `new Foo Bar()`, was enough). The syntax errors ARE the outcome — skip
    // semantic analysis, as the backend below is already skipped, and exit
    // non-zero via getNumErrors().
    if (!Diags.hasErrorOccurred())
        SemaModules = S->Resolve(ASTModules, CI.getCodeGenOptions().TestMode);

    // Never lower modules that carry parse/sema errors: a failed resolution
    // leaves null types/symbols behind and CodeGen dereferences them — the
    // compiler crashed (0xC0000005) AFTER the diagnostics were printed. The
    // errors are the outcome; skip the backend and exit non-zero (see the
    // getNumErrors() return below).
    if (!SemaModules.empty() && !Diags.hasErrorOccurred()) {

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
        const BackendActionKind Action = CI.getFrontendOptions().BackendAction;
        const bool IsEmitAction = Action == BackendActionKind::Backend_EmitLL ||
                                  Action == BackendActionKind::Backend_EmitBC ||
                                  Action == BackendActionKind::Backend_EmitAssembly;
        const std::string &ExplicitOut = CI.getFrontendOptions().getOutputFile();

        // A single explicit -o output means a single artifact: lower all input files into
        // one module so cross-file references resolve. Without -o, each file is emitted to
        // its own .ll/.bc/.s/.o.
        //
        // An emit action (--emit-ll/-bc/-as) that pulled dependency modules through
        // imports (--src-dir / project root) MUST also use the single-module path even
        // without -o: those modules reference each other, and per-file lowering leaves a
        // call to a sibling module's function as a bodyless stub whose CodeGen is null
        // (crash). Explicitly listed inputs with no resolved deps keep per-file emission.
        bool SingleModule = !ExplicitOut.empty() || (IsEmitAction && PulledSourceDeps);
        llvm::SmallVector<llvm::Module *, 8> Modules = CG.GenerateModules(CompilableModules, SingleModule);

        // Emit code base on BackendActionKind
        const std::string &OutDir = CI.getFrontendOptions().OutDir;
        for (auto M : Modules) {
            // M->getName() is the source filename (e.g. "main.fly"); getOutputFileName
            // turns it into "main.fly.o" / "main.fly.ll" etc. Redirect under --out-dir
            // when set, and pass the resolved path to Emit so it lands there.
            //
            // -o names the artifact of the LAST stage actually performed. When there is
            // no link step (--emit-ll/-bc/-as or -c) that stage is this one, so write
            // straight to it — the path was already resolved under --out-dir above. When
            // linking, -o belongs to the linked artifact and the intermediate object
            // keeps its derived name.
            //
            // -o needs no extension: the one implied by the format is appended when
            // absent, so `--emit-ll -o out` writes out.ll and `-c -o out` writes out.o.
            // An extension the user did spell out is left alone (`-o out.txt` stays
            // out.txt) — same rule --lib has always followed for the archive name.
            const bool NamesThisArtifact =
                !CI.getFrontendOptions().LinkStep && !ExplicitOut.empty();
            std::string OutFile = NamesThisArtifact
                                      ? ExplicitOut
                                      : underOutDir(OutDir, CG.getOutputFileName(M->getName()));
            if (NamesThisArtifact && llvm::sys::path::extension(OutFile).empty())
                OutFile += CodeGen::getOutputExtension(Action).str();
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
    bool hasOutput = !FO.getOutputFile().empty();
    // Discovery picks the stem when it knows a better name than the entry file's
    // (the suite name, or the source-root directory for library/multi-suite builds).
    std::string stem = FO.DefaultOutputStem.empty()
        ? llvm::sys::path::stem(llvm::sys::path::filename(FO.getInputFiles()[0])).str()
        : FO.DefaultOutputStem;

    // Forced library (--lib/--lib-dyn): keep the library behaviour set by the Driver,
    // even when a main() is present. Only auto-name the output (ToolChain appends the
    // platform extension: .a/.lib for static, .so/.dylib/.dll for shared).
    if (FO.CreateLibrary || FO.CreateSharedLib) {
        if (!hasOutput) FO.setOutputFile(stem, /*isLib=*/true);
        FLY_DEBUG_MSG("Forced library output '" << stem << "'");
        return;
    }

    // Precedence: suite → test exe; main → exe; else → lib.
    if (hasSuite) {
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

    // The project source root: the current directory by default, overridden by a
    // single --src-dir.
    llvm::SmallVector<std::string, 4> Dirs = FO.SrcDirs;
    if (Dirs.empty())
        Dirs.push_back(".");

    // -L dirs are source roots too: their bare .fly files are no longer loaded as
    // declaration-only headers (see LoadLibHeaders SkipBareSources), so an import
    // served by neither a .fly.h nor the project sources is pulled FROM THE -L DIR
    // in full and compiled in. Namespaces that DO have a .fly.h stay header-served
    // (the archive beside the header defines them) — the HeaderNs guard below.
    for (const auto &D : FO.LibDirs)
        Dirs.push_back(D);

    // Namespaces already served by parsed HEADER modules (.fly.h from the stdlib
    // and -L dirs): their symbols link from compiled archives — never pull their
    // sources, even when a source tree with the same namespaces is in reach.
    // Files the command line already lists. ResolveSourceDeps runs BEFORE the remaining
    // explicit inputs are parsed, so an import can resolve to a file the user listed
    // anyway: pulling one of those is NOT a hidden dependency, and must not switch an
    // emit build to the combined-module path (the user asked for per-file emission).
    llvm::StringSet<> ExplicitInputs;
    for (const auto &In : FO.getInputFiles())
        ExplicitInputs.insert(llvm::sys::path::filename(In));

    llvm::StringSet<> HeaderNs;
    for (auto *M : ASTModules)
        if (M->isHeader() && M->getNameSpace()) {
            std::string ns;
            for (const auto *N : M->getNameSpace()->getNames()) {
                if (!ns.empty()) ns += ".";
                ns += N->getName().str();
            }
            HeaderNs.insert(ns);
        }

    // Namespace → [path] map, built LAZILY: the recursive walk only happens when an
    // import actually needs project sources, so std-only programs never pay for it.
    std::map<std::string, std::vector<std::string>> NsToFiles;
    llvm::StringSet<> KnownFiles; // files already parsed (entry + headers)
    bool Scanned = false;

    // BFS: for each parsed module, resolve its imports to source files and parse them.
    // ParseFile appends to ASTModules, so newly added modules are visited in turn.
    size_t i = 0;
    while (i < ASTModules.size()) {
        ASTModule *M = ASTModules[i++];
        for (ASTNode *N : M->getNodes()) {
            if (N->getKind() != ASTKind::AST_IMPORT) continue;
            auto *Imp = static_cast<ASTImport *>(N);
            std::string ns = importNamespace(Imp);

            // Header-served import (matched as-is, or — for plain `Namespace.Symbol`
            // imports — via a parent namespace): nothing to pull from source.
            bool HeaderServed = HeaderNs.count(ns) > 0;
            if (!HeaderServed && !Imp->isWildcard()) {
                std::string parent = ns;
                while (!HeaderServed) {
                    auto dot = parent.rfind('.');
                    if (dot == std::string::npos) break;
                    parent = parent.substr(0, dot);
                    HeaderServed = HeaderNs.count(parent) > 0;
                }
            }
            if (HeaderServed) continue;

            if (!Scanned) {
                Scanned = true;
                for (const auto *PM : ASTModules)
                    KnownFiles.insert(llvm::sys::path::filename(PM->getFile()->getFileName()));
                std::error_code EC;
                for (const auto &Dir : Dirs) {
                    for (llvm::sys::fs::recursive_directory_iterator I2(Dir, EC), E2;
                         I2 != E2 && !EC; I2.increment(EC)) {
                        const std::string &Path = I2->path();
                        llvm::StringRef PathRef(Path);
                        if (llvm::sys::fs::is_directory(Path)) {
                            if (DiscoverySkipsDir(llvm::sys::path::filename(Path)))
                                I2.no_push();
                            continue;
                        }
                        if (!PathRef.ends_with(".fly") || PathRef.ends_with(".fly.h")) continue;
                        if (KnownFiles.count(llvm::sys::path::filename(Path))) continue;
                        std::string fns = extractFileNamespace(Path);
                        if (!fns.empty()) NsToFiles[fns].push_back(Path);
                    }
                }
            }

            // Namespaces to pull: the imported one, or — ONLY when no file
            // declares that exact namespace — its DESCENDANTS (an `import my` /
            // `import my.*` must discover my.utils from source; in directory mode
            // nothing else brings those files in). When the exact namespace IS
            // declared, its files alone are the import's meaning: pulling
            // descendants too would drag sibling trees in (e.g. the compiler's
            // own `fly.compiler.codegen.*` TEST suites into a compiler build).
            // Header-served descendants stay with their archives either way.
            llvm::SmallVector<std::string, 4> MatchNs;
            if (NsToFiles.count(ns)) {
                MatchNs.push_back(ns);
            } else {
                const std::string Prefix = ns + ".";
                for (const auto &Entry : NsToFiles)
                    if (llvm::StringRef(Entry.first).starts_with(Prefix) &&
                        !HeaderNs.count(Entry.first))
                        MatchNs.push_back(Entry.first);
            }
            // A plain/alias import is `Namespace.Symbol` (e.g. `import fly.compiler.ast.ASTNode`):
            // its trailing component is the imported class/enum/function name, NOT a namespace
            // component, so the joined path is not a declared namespace. Fall back to the parent
            // namespace (drop the last component) so the namespace's source files still get pulled
            // in. Wildcard imports already carry the bare namespace, so skip them here.
            if (MatchNs.empty() && !Imp->isWildcard()) {
                std::string parent = ns;
                while (MatchNs.empty()) {
                    auto dot = parent.rfind('.');
                    if (dot == std::string::npos) break;
                    parent = parent.substr(0, dot);
                    if (NsToFiles.count(parent))
                        MatchNs.push_back(parent);
                }
            }
            if (MatchNs.empty()) continue;
            for (const auto &MN : MatchNs) {
                auto it = NsToFiles.find(MN);
                for (const auto &Path : it->second) {
                    llvm::StringRef Fname = llvm::sys::path::filename(Path);
                    if (KnownFiles.count(Fname)) continue;
                    KnownFiles.insert(Fname);
                    FLY_DEBUG_MSG("Resolved import '" << ns << "' → " << Path);
                    Diags.getClient()->BeginSourceFile();
                    ParseFile(Builder, Path);
                    Diags.getClient()->EndSourceFile();
                    // A module the command line did NOT list was pulled in: it and the
                    // importer reference each other, so they cannot be lowered per-file.
                    // A file the user listed anyway keeps the explicit per-file behaviour.
                    if (!ExplicitInputs.count(Fname))
                        PulledSourceDeps = true;
                }
                // Done with this namespace; avoid re-processing it for other modules.
                NsToFiles.erase(it);
            }
        }
    }
}

// Token-level scan of a .fly file for its TOP-LEVEL declarations: a `main`
// identifier followed by `(` at brace depth 0 is the program entry point, and
// `suite <Name>` at depth 0 declares a test suite. The real Lexer is used (it
// resolves keywords and skips comments/strings), but nothing is parsed — class
// methods named main sit at depth >= 1 and are never mistaken for the entry.
static void ScanTopLevelDecls(SourceManager &SM, const std::string &Path,
                              bool &HasMain,
                              llvm::SmallVectorImpl<std::string> &Suites) {
    HasMain = false;
    auto MBOrErr = llvm::MemoryBuffer::getFile(Path);
    if (!MBOrErr)
        return;
    const llvm::MemoryBuffer *MB = MBOrErr.get().get();
    const FileID FID = SM.createFileID(std::move(MBOrErr.get()));
    Lexer Lex(FID, MB, SM);

    unsigned Depth = 0;
    bool PrevIsMain = false, PrevIsSuiteKw = false;
    Token Tok;
    while (true) {
        Lex.Lex(Tok);
        if (Tok.is(tok::eof))
            break;
        if (Tok.is(tok::l_brace)) {
            ++Depth;
            PrevIsMain = PrevIsSuiteKw = false;
            continue;
        }
        if (Tok.is(tok::r_brace)) {
            if (Depth) --Depth;
            PrevIsMain = PrevIsSuiteKw = false;
            continue;
        }
        if (Depth != 0) {
            PrevIsMain = PrevIsSuiteKw = false;
            continue;
        }
        if (PrevIsSuiteKw && Tok.is(tok::identifier))
            Suites.push_back(Tok.getIdentifierInfo()->getName().str());
        if (PrevIsMain && Tok.is(tok::l_paren))
            HasMain = true;
        PrevIsSuiteKw = Tok.is(tok::kw_suite);
        PrevIsMain = Tok.is(tok::identifier) &&
                     Tok.getIdentifierInfo()->getName() == "main";
    }
}

bool Frontend::DiscoverInputs() {
    FLY_DEBUG_SCOPE("Frontend", "DiscoverInputs");
    FrontendOptions &FO = CI.getFrontendOptions();
    const std::string Root = FO.SrcDirs.empty() ? std::string(".") : FO.SrcDirs[0];

    // Every .fly source under the root (recursive; .fly.h are headers, not inputs).
    // Sorted so the entry order — and with it module order and output naming — is
    // deterministic across filesystems.
    std::vector<std::string> Files;
    std::error_code EC;
    for (llvm::sys::fs::recursive_directory_iterator I(Root, EC), E;
         I != E && !EC; I.increment(EC)) {
        llvm::StringRef P(I->path());
        if (llvm::sys::fs::is_directory(P)) {
            if (DiscoverySkipsDir(llvm::sys::path::filename(P)))
                I.no_push();
            continue;
        }
        if (P.ends_with(".fly"))
            Files.push_back(I->path());
    }
    std::sort(Files.begin(), Files.end());
    if (Files.empty()) {
        Diags.Report(diag::err_fe_no_sources) << Root;
        return false;
    }

    // The source-root directory name, for outputs that no single file can name
    // (whole-directory library, multi-suite test executable). "." resolves to the
    // actual directory the build runs in.
    auto RootStem = [&Root]() {
        llvm::SmallString<256> Abs(Root);
        llvm::sys::fs::make_absolute(Abs);
        llvm::sys::path::remove_dots(Abs, /*remove_dot_dot=*/true);
        return std::string(llvm::sys::path::filename(Abs));
    };

    // Library builds compile the whole directory: the directory IS the library.
    if (FO.CreateLibrary || FO.CreateSharedLib) {
        for (const auto &F : Files)
            FO.addInputFile(F.c_str());
        FO.DefaultOutputStem = RootStem();
        return true;
    }

    // Scan the top-level declarations of every source.
    llvm::SmallVector<std::string, 8> MainFiles;
    std::vector<std::pair<std::string, std::string>> SuiteDecls; // (suite, file)
    for (const auto &F : Files) {
        bool HasMain = false;
        llvm::SmallVector<std::string, 4> Suites;
        ScanTopLevelDecls(CI.getSourceManager(), F, HasMain, Suites);
        if (HasMain)
            MainFiles.push_back(F);
        for (const auto &S : Suites)
            SuiteDecls.emplace_back(S, F);
        FLY_DEBUG_MSG("Scanned " << F << ": main=" << HasMain
                                 << " suites=" << Suites.size());
    }

    // A linking root with no main() and exactly one suite: that suite is the entry.
    // Each suite gets its own implicit main(), so more than one cannot be linked
    // together. Non-linking stages fall through to the whole-directory rule below.
    if (FO.LinkStep && MainFiles.empty() && !SuiteDecls.empty()) {
        if (SuiteDecls.size() > 1) {
            std::string List;
            for (const auto &SD : SuiteDecls) {
                if (!List.empty())
                    List += ", ";
                List += SD.first;
            }
            Diags.Report(diag::err_fe_multiple_suites) << Root << List;
            return false;
        }
        FO.addInputFile(SuiteDecls[0].second.c_str());
        FO.DefaultOutputStem = SuiteDecls[0].first;
        return true;
    }

    // One top-level main() under the root selects the entry (its import closure
    // pulls the rest) — for the non-linking stages too, so `-c --src-dir <proj>`
    // compiles the PROGRAM, not every stray test source in the tree. With no
    // main at all, a non-linking build has no entry to choose and compiles the
    // whole directory; a linking build has nothing to link.
    if (MainFiles.empty()) {
        if (!FO.LinkStep) {
            for (const auto &F : Files)
                FO.addInputFile(F.c_str());
            FO.DefaultOutputStem = RootStem();
            return true;
        }
        Diags.Report(diag::err_fe_no_main) << Root;
        return false;
    }
    if (MainFiles.size() > 1) {
        std::string List;
        for (const auto &F : MainFiles) {
            if (!List.empty())
                List += ", ";
            List += F;
        }
        Diags.Report(diag::err_fe_multiple_main) << Root << List;
        return false;
    }
    FO.addInputFile(MainFiles[0].c_str());
    // DefaultOutputStem stays empty: the executable is named after the main file.
    return true;
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
                              bool preferDotFlyH, bool SkipBareSources) {
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

        // Skip directories silently — the iterator descends into them
        // automatically, EXCEPT the B009 skip-list (dot-dirs and build/).
        if (llvm::sys::fs::is_directory(Path)) {
            if (DiscoverySkipsDir(Filename))
                I.no_push();
            continue;
        }

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

        // -L dirs (SkipBareSources): a bare .fly is a SOURCE, not a header.
        // Loading it declaration-only left every symbol undefined at link when no
        // archive ships beside it (the seed compiling the 0.14 std suites).
        // ResolveSourceDeps walks the -L dirs, so the namespace is pulled in full
        // when — and only when — something imports it.
        if (SkipBareSources)
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
