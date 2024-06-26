//===--------------------------------------------------------------------------------------------------------------===//
// src/Frontend/Frontend.cpp - Main Compiler Frontend
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Frontend/Frontend.h"
#include "Sema/Sema.h"
#include "Sema/SemaBuilder.h"
#include "AST/ASTNameSpace.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "Basic/Archiver.h"
#include "Basic/Debug.h"
#include "Basic/Stack.h"

#include <llvm/ADT/Statistic.h>
#include <llvm/Support/Timer.h>

#include <iostream>

using namespace fly;

Frontend::Frontend(CompilerInstance &CI) : CI(CI), Diags(CI.getDiagnostics()) {

}

Frontend::~Frontend() {

}

bool Frontend::Execute() {
    assert(!CI.getFrontendOptions().ShowHelp && "Client must handle '-help'!");
    assert(!CI.getFrontendOptions().ShowVersion && "Client must handle '-version'!");
    FLY_DEBUG("Frontend", "Execute");

    raw_ostream &OS = llvm::errs();

    // Create Timers and show after compilation
    if (CI.getFrontendOptions().ShowTimers)
        CreateFrontendTimer();

    if (CI.getFrontendOptions().ShowStats)
        llvm::EnableStatistics(false);

    // Generate Backend Code
    CodeGen CG(Diags, CI.getCodeGenOptions(), CI.getTargetOptions(),
               CI.getFrontendOptions().BackendAction,
               CI.getFrontendOptions().ShowTimers);

    // Parse files, create AST, build Semantics checker
    Sema *S = Sema::CreateSema(Diags);
    std::vector<FrontendAction *> Actions = ParseActions(CG, *S);
    if (!Actions.empty() && S->Resolve()) {

        // Generate Code for Top Definitions
        for (auto Action: Actions) {
            Action->GenerateTopDef();
        }

        // Generate Code for Bodies
        for (auto Action: Actions) {
            if (!Action->GenerateBodies()) {
                break;
            }
            if (!Action->HandleTranslationUnit()) {
                break;
            }
            if (!Action->getHeaderFile().empty()) { // Add Header File if exists to output files
                OutputFiles.push_back(Action->getHeaderFile());
            }
            OutputFiles.push_back(Action->getOutputFile());
        }
    }

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

    delete S;

    return !CI.getDiagnostics().getClient()->getNumErrors();
}

/**
 * Read Input Files from options and add to Actions if parsing is true.
 * @param CG
 * @return
 */
std::vector<FrontendAction *> Frontend::ParseActions(CodeGen &CG, Sema &S) {
    std::vector<FrontendAction *> Actions;

    if (CI.getFrontendOptions().getInputFiles().empty()) {
        Diags.Report(SourceLocation(), diag::note_no_input_process);
        return Actions;
    }

    for (auto &InputFileName : CI.getFrontendOptions().getInputFiles()) {
        FLY_DEBUG_MESSAGE("Frontend", "Execute", "Loading input file " + InputFileName);
        InputFile *Input = new InputFile(Diags, CI.getSourceManager(), InputFileName);
        if (Input->getExt() == FileExt::FLY) {
            if (Input->Load()) {
                FrontendAction *Action = new FrontendAction(CI, CG, S, Input);
                // Parse Action & add to Actions for next
                if (Action->Parse()) {
                    Actions.emplace_back(Action);
                }
            }
        } else if (Input->getExt() == FileExt::LIB) {
            // Read Header Files from library by extracting them
            const std::vector<StringRef> Files = ExtractFiles(InputFileName);
            for (StringRef File : Files) {
                if (llvm::sys::path::extension(File) == ".h") {
                    InputFile *InputHeader = new InputFile(Diags, CI.getSourceManager(), File.str());
                    if (InputHeader->Load()) {
                        FrontendAction *Action = new FrontendAction(CI, CG, S, InputHeader);
                        Action->ParseHeader();
                    }

                    // Remove Header File after extraction and parsing
                    const std::error_code EC = llvm::sys::fs::remove(File, false);
                    if (EC) {
                        Diags.Report(diag::err_fe_unable_remove_header) << EC.message();
                    }
                }
            }
        } else {
            CI.getDiagnostics().Report(diag::err_fe_input_file_ext) << InputFileName;
        }
    }
    return Actions;
}

void Frontend::CreateFrontendTimer() {
    FLY_DEBUG("Frontend", "CreateFrontendTimer");
    FrontendTimerGroup.reset(
            new llvm::TimerGroup("frontend", "Clang front-end time report"));
    FrontendTimer.reset(
            new llvm::Timer("frontend", "Clang front-end timer",
                            *FrontendTimerGroup));
}

const SmallVector<std::string, 4> &Frontend::getOutputFiles() const {
    return OutputFiles;
}

std::vector<StringRef> Frontend::ExtractFiles(const std::string &LibFileName) {
    Archiver Ar(Diags, LibFileName);
    std::vector<StringRef> List;
    if (Ar.ExtractLib(CI.getFileManager())) {
        return Ar.getExtractFiles();
    }
    return List;
}
