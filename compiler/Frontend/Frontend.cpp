//===--------------------------------------------------------------------------------------------------------------===//
// src/Frontend/Frontend.cpp - Main Compiler Frontend
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Frontend/Frontend.h"

#include "AST/ASTBuilder.h"
#include "Frontend/InputFile.h"
#include "Basic/Archiver.h"
#include "Basic/Debug.h"
#include "Basic/Stack.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "Parser/Parser.h"
#include "Sema/SemaBuiltin.h"
#include "Sema/SemaContext.h"

#include <iostream>
#include <llvm/ADT/Statistic.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Timer.h>

using namespace fly;

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
        Diags.Report(SourceLocation(), diag::note_no_input_process);
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
        llvm::SmallVector<llvm::Module *, 8> Modules = CG.GenerateModules(SemaModules);

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

    	// FIXME: Generate Headers
        // if (CI.getFrontendOptions().CreateHeader) {
        //     CG.GenerateHeaders(S->getSymTable());
        // }
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
        // Read Header Files from library by extracting them
        const std::vector<StringRef> Files = ExtractFiles(FileName);
        for (StringRef File : Files) {
            if (llvm::sys::path::extension(File) == ".fly.h") {
                InputFile *InputHeader = new InputFile(Diags, CI.getSourceManager(), File.str());
                if (InputHeader->Load()) {
                    Parser *P = new Parser(InputHeader, CI.getSourceManager(), Diags, Builder);
                    P->ParseHeader();
                    delete P;
                }
                delete InputHeader;

                // Remove Header File after extraction and parsing
                const std::error_code EC = llvm::sys::fs::remove(File, false);
                if (EC) {
                    Diags.Report(diag::err_fe_unable_remove_header) << EC.message();
                }
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
    for (llvm::sys::fs::directory_iterator I(FLY_LIB_FLY_DIR, EC), E;
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
