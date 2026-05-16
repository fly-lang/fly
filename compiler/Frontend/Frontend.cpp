//===--------------------------------------------------------------------------------------------------------------===//
// src/Frontend/Frontend.cpp - Main Compiler Frontend
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

#include <iostream>
#include <llvm/ADT/Statistic.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Timer.h>

using namespace fly;

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
static std::string GenerateHeader(ASTModule *M, DiagnosticsEngine &Diags) {
    // Derive header path: <source>.fly → <source>.fly.h
    std::string HeaderPath = M->getName() + ".fly.h";

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
            OS << "    " << Meth->getName().str() << "(";
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
                OS << "    " << typeStr(V->getType()) << " " << V->getName() << "\n";
            } else if (Child->getKind() == ASTKind::AST_FUNCTION) {
                // Methods use AST_FUNCTION kind (ASTMethod extends ASTFunction)
                const auto *Meth = static_cast<const ASTMethod *>(Child);
                // Skip constructors (name matches class name)
                if (Meth->getName() == C->getName()) continue;
                bool methPublic = false;
                for (auto *Mod : Meth->getModifiers())
                    if (Mod->getModifierKind() == ASTModifierKind::MOD_PUBLIC)
                        methPublic = true;
                if (!methPublic) continue;
                OS << "    public " << Meth->getName().str() << "(";
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
    FLY_DEBUG_START("Frontend", "Execute");

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

    // Load fly stdlib headers before user files so their declarations
    // are available when resolving imports like "import fly.string".
#ifdef FLY_LIB_FLY_DIR
    LoadStdlibHeaders(*Builder);
#endif

    for (auto &FileName: CI.getFrontendOptions().getInputFiles()) {
        Diags.getClient()->BeginSourceFile();
        ParseFile(*Builder, FileName);
        Diags.getClient()->EndSourceFile();
    }

	// Parse files, create AST, build Semantics checker
	SemaContext *S = new SemaContext(Diags);

    Diags.getClient()->BeginSourceFile();

    // Resolve AST references
	SmallVector<SemaModule *, 8> SemaModules = S->Resolve(ASTModules);

    if (!SemaModules.empty()) {

        // Create LLVM Context (must outlive all modules)
        llvm::LLVMContext LLVMCtx;

        // Generate Backend Code
        // Parsers and InputFiles must remain alive through CodeGen: AST StringRefs
        // (class/function names, type names, etc.) point into the Lexer's IdentifierTable.
        CodeGen CG(Diags, LLVMCtx, CI.getCodeGenOptions(), CI.getTargetOptions(),
                   CI.getFrontendOptions().BackendAction,
                   CI.getFrontendOptions().ShowTimers);
        SmallVector<SemaModule *, 8> CompilableModules;
        for (auto *SM : SemaModules)
            if (!SM->getAST().isHeader())
                CompilableModules.push_back(SM);
        llvm::SmallVector<llvm::Module *, 8> Modules = CG.GenerateModules(CompilableModules);

        // Emit code base on BackendActionKind
        for (auto M : Modules) {
            // Pass empty OutName: Emit will call getOutputFileName(M->getName())
            // where M->getName() is the source filename (e.g. "main.fly"),
            // producing the correct output path (e.g. "main.fly.o" / "main.fly.ll" etc.)
            std::string OutFile = CG.getOutputFileName(M->getName());
            CG.Emit(M, "");
            if (!OutFile.empty())
                OutputFiles.push_back(OutFile);
        }

    	// Delete generated LLVM modules (ownership transferred from CodeGenModule here)
    	for (auto *M : Modules)
    		delete M;
    	Modules.clear();

        // Generate .fly.h headers for each non-header module when --lib/--header is set.
        if (CI.getFrontendOptions().CreateHeader) {
            for (auto *SM : CompilableModules) {
                std::string HPath = GenerateHeader(&SM->getAST(), Diags);
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

/**
 * ParseModule Input File.
 * @param CG
 * @return
 */
void Frontend::ParseFile(ASTBuilder &Builder, const std::string &FileName) {
    FLY_DEBUG_START_MSG("Frontend", "Execute", "Loading input file " + FileName);
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
    FLY_DEBUG_START("Frontend", "CreateFrontendTimer");
    FrontendTimerGroup.reset(
            new llvm::TimerGroup("frontend", "Clang front-end time report"));
    FrontendTimer.reset(
            new llvm::Timer("frontend", "Clang front-end timer",
                            *FrontendTimerGroup));
}

const SmallVector<std::string, 4> &Frontend::getOutputFiles() const {
    return OutputFiles;
}

#ifdef FLY_LIB_FLY_DIR
void Frontend::LoadStdlibHeaders(ASTBuilder &Builder) {
    std::error_code EC;
    for (llvm::sys::fs::recursive_directory_iterator I(FLY_LIB_FLY_DIR, EC), E;
         I != E && !EC; I.increment(EC)) {
        const std::string &Path = I->path();
        // Accept files ending in ".fly.h"
        if (Path.size() > 6 && Path.substr(Path.size() - 6) == ".fly.h") {
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
}
#endif

std::vector<StringRef> Frontend::ExtractFiles(const std::string &LibFileName) {
    Archiver Ar(Diags, LibFileName);
    std::vector<StringRef> List;
    if (Ar.ExtractLib(CI.getFileManager())) {
        return Ar.getExtractFiles();
    }
    return List;
}

bool Archiver::ExtractLib(FileManager &FileMgr) {
    FLY_DEBUG_START("Archiver", "ExtractLib");
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
