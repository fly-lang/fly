//===----------------------------------------------------------------------===//
// Compiler/Compiler.cpp - Main Compiler
//
// Part of the Fly Project, under the Apache License v2.0
// See https://flylang.org/LICENSE.txt for license information.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//
#include "Compiler/Compiler.h"
#include "Compiler/ChainedDiagnosticConsumer.h"
#include "Compiler/LogDiagnosticPrinter.h"

using namespace fly;
using namespace llvm;

Compiler::Compiler(int argc, const char **argv) : Compiler(getPath(argv[0]), getArgs(argc, argv)) {}

Compiler::Compiler(const std::string& path, ArrayRef<const char *> argList) {

    // Create Diagnostics
    createDiagnostics();

    invocation = new CompilerInvocation(getDiagnostics(), path, argList);

    // Create Compiler Instance for each input file
    for (const auto &InputFile : invocation->inputFiles) {
        instances.push_back(new CompilerInstance(*invocation, InputFile));
    }
}

// This function isn't referenced outside its translation unit, but it
// can't use the "static" keyword because its address is used for
// GetMainExecutable (since some platforms don't support taking the
// address of main, and some platforms can't implement GetMainExecutable
// without being given the address of a function in the main executable).
std::string Compiler::GetExecutablePath(const char *argv0, void *mainAddr) {
    return llvm::sys::fs::getMainExecutable(argv0, mainAddr);
}

std::string Compiler::getPath(const char * argv0) {
    // This just needs to be some symbol in the binary; C++ doesn't
    // allow taking the address of ::main however.
    void *mainAddr = (void*) (intptr_t) GetExecutablePath;
    return GetExecutablePath(argv0, mainAddr);
}

SmallVector<const char *, 16> Compiler::getArgs(int argc, const char **argv) {
    SmallVector<const char *, 16> args(argv, argv + argc);
    return std::move(args);
}

Compiler::~Compiler() {
    //assert(OutputFiles.empty() && "Still output files in flight?");
    // TODO delete CompilerInvocation
    // Delete Compiler Instances
}

bool Compiler::execute() const {
    bool res = true;
    for (auto &instance : instances) {
        res &= instance->execute();
    }
    return res;
}

static void SetUpDiagnosticLog(DiagnosticOptions *diagOpts,
                               DiagnosticsEngine &diags) {
    std::error_code EC;
    std::unique_ptr<raw_ostream> StreamOwner;
    raw_ostream *OS = &llvm::errs();
    if (diagOpts->DiagnosticLogFile != "-") {
        // Create the output stream.
        auto FileOS = std::make_unique<llvm::raw_fd_ostream>(
                diagOpts->DiagnosticLogFile, EC,
                llvm::sys::fs::OF_Append | llvm::sys::fs::OF_Text);
        if (EC) {
            diags.Report(diag::warn_fe_cc_log_diagnostics_failure)
                    << diagOpts->DiagnosticLogFile << EC.message();
        } else {
            FileOS->SetUnbuffered();
            OS = FileOS.get();
            StreamOwner = std::move(FileOS);
        }
    }

    // Chain in the diagnostic client which will log the diagnostics.
    auto Logger = std::make_unique<LogDiagnosticPrinter>(*OS, diagOpts,
                                                         std::move(StreamOwner));

    if (diags.ownsClient()) {
        diags.setClient(
                new ChainedDiagnosticConsumer(diags.takeClient(), std::move(Logger)));
    } else {
        diags.setClient(
                new ChainedDiagnosticConsumer(diags.getClient(), std::move(Logger)));
    }
}

// Diagnostics

void Compiler::createDiagnostics(DiagnosticConsumer *client,
                                           bool shouldOwnClient) {
    DiagnosticOptions *DiagOpts = new DiagnosticOptions();
    IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
    diagnostics = new DiagnosticsEngine(DiagID, DiagOpts);

    // Create the diagnostic client for reporting errors or for
    // implementing -verify.
    if (client) {
        diagnostics->setClient(client, shouldOwnClient);
    } else
        diagnostics->setClient(new TextDiagnosticPrinter(llvm::errs(), DiagOpts));

    // Chain in -diagnostic-log-file dumper, if requested.
    if (!DiagOpts->DiagnosticLogFile.empty())
        SetUpDiagnosticLog(DiagOpts, *diagnostics);

    // Configure our handling of diagnostics.
    ProcessWarningOptions(*diagnostics, *DiagOpts);
}

CompilerInvocation &Compiler::getInvocation() {
    return *invocation;
}

const std::vector<CompilerInstance *> &Compiler::getInstances() const {
    return instances;
}
