//===--------------------------------------------------------------------------------------------------------------===//
// src/Frontend/Frontend.cpp - Main Compiler Frontend
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "Frontend/Frontend.h"
#include "CodeGen/CodeGen.h"
#include "Basic/Debug.h"
#include <llvm/ADT/Statistic.h>
#include <llvm/Support/Timer.h>
#include <Basic/Stack.h>

using namespace fly;

Frontend::Frontend(CompilerInstance &CI) : CI(CI), Diags(CI.getDiagnostics()), Context(new ASTContext(Diags)) {

}

Frontend::~Frontend() {
    delete Context;
}

bool Frontend::Execute() {
    assert(!CI.getFrontendOptions().ShowHelp && "Client must handle '-help'!");
    assert(!CI.getFrontendOptions().ShowVersion && "Client must handle '-version'!");
    FLY_DEBUG("Frontend", "Execute");

    unsigned NumberOfInputs = 0;
    raw_ostream &OS = llvm::errs();

    // Create Timers and show after compilation
    if (CI.getFrontendOptions().ShowTimers)
        CreateFrontendTimer();

    if (CI.getFrontendOptions().ShowStats)
        llvm::EnableStatistics(false);

    // Generate Backend Code
    CodeGen CG(Diags, CI.getCodeGenOptions(), CI.getTargetOptions(),
               CI.getFrontendOptions().BackendAction, CI.getFrontendOptions().ShowTimers);

    // Create Compiler Instance for each input file
    for (auto InputFile : CI.getFrontendOptions().getInputFiles()) {
        // Print file name and create instance for file compilation

        FLY_DEBUG_MESSAGE("Frontend", "Execute", "Loading input file " << llvm::sys::path::filename(InputFile.getFile()) << "\n");
        if (InputFile.Load(CI.getSourceManager(), Diags)) {
            FrontendAction *Action = new FrontendAction(CI, Context, CG);
            // Parse Action & add to Actions for next
            if (Action->Parse(InputFile)) {
                Actions.emplace_back(Action);
                NumberOfInputs++;
            }
        }
    }

    if (NumberOfInputs > 0) {
        if (Context->Resolve()) {
            llvm::outs().flush();
            for (auto Action: Actions) {
                Action->Compile() && Action->EmitOutput();
            }
        }
    } else {
        Diags.Report(SourceLocation(), diag::note_no_input_process);
    }

    Diags.getClient()->finish();

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

    return !CI.getDiagnostics().getClient()->getNumErrors();
}

void Frontend::CreateFrontendTimer() {
    FLY_DEBUG("Frontend", "CreateFrontendTimer");
    FrontendTimerGroup.reset(
            new llvm::TimerGroup("frontend", "Clang front-end time report"));
    FrontendTimer.reset(
            new llvm::Timer("frontend", "Clang front-end timer",
                            *FrontendTimerGroup));
}
