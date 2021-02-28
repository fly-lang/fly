//
// Created by marco on 2/27/21.
//

#include "Driver/Driver.h"
#include "Driver/DriverOptions.h"
#include "Frontend/Frontend.h"
#include "Frontend/ChainedDiagnosticConsumer.h"
#include "Frontend/LogDiagnosticPrinter.h"
#include <llvm/Support/FileSystem.h>

using namespace fly;

// This function isn't referenced outside its translation unit, but it
// can't use the "static" keyword because its address is used for
// GetMainExecutable (since some platforms don't support taking the
// address of main, and some platforms can't implement GetMainExecutable
// without being given the address of a function in the main executable).
std::string GetExecutablePath(const char *argv0, void *mainAddr) {
    return llvm::sys::fs::getMainExecutable(argv0, mainAddr);
}

std::string getCurPath(const char *argv0) {
    // This just needs to be some symbol in the binary; C++ doesn't
    // allow taking the address of ::main however.
    void *mainAddr = (void *) (intptr_t) GetExecutablePath;
    return GetExecutablePath(argv0, mainAddr);
}

llvm::SmallVector<const char *, 16> getArgs(int argc, const char **argv) {
    llvm::SmallVector<const char *, 16> args(argv, argv + argc);
    return std::move(args);
}

llvm::opt::InputArgList parseArgStrings(llvm::ArrayRef<const char *> ArgStrings) {
    unsigned MissingArgIndex, MissingArgCount;
    return fly::driver::getDriverOptTable().ParseArgs(ArgStrings, MissingArgIndex, MissingArgCount);
}


void SetUpDiagnosticLog(DiagnosticOptions *diagOpts,
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

Driver::Driver() : Driver("fly", {"fly"}) {}

Driver::Driver(int argc, const char **argv) : Driver(getCurPath(argv[0]), getArgs(argc, argv)) {}

Driver::Driver(const std::string& path, llvm::ArrayRef<const char *> args) :
        path(path), argList(std::move(parseArgStrings(args))) {
    createDiagnostics();
    createInvocation();
}

// Diagnostics

void Driver::createDiagnostics(DiagnosticConsumer *client,
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
        
const std::string &Driver::getPath() const {
    return path;
}

const llvm::opt::InputArgList &Driver::getArgList() const {
    return argList;
}

void Driver::createInvocation() {

    std::shared_ptr<TargetOptions> targetOpts(new TargetOptions);
    targetOpts->Triple = "x86_64-GNU-Linux";
    IntrusiveRefCntPtr<TargetInfo> target = TargetInfo::CreateTargetInfo(*diagnostics, targetOpts);
    std::shared_ptr<FrontendOptions> frontendOptions;

    // Create Diagnostics
    createDiagnostics();

    invocation = std::make_shared<CompilerInvocation>(diagnostics, frontendOptions, target);
}

bool Driver::execute() {

    Frontend frontend(*invocation);
}

const IntrusiveRefCntPtr<DiagnosticsEngine> &Driver::getDiagnostics() const {
    return diagnostics;
}

const std::shared_ptr<CompilerInvocation> &Driver::getInvocation() const {
    return invocation;
}
