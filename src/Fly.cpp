//===--------------------------------------------------------------------------------------------------------------===//
// src/Fly.cpp - Main
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include <Config/config.h>
#include "Driver/Driver.h"
#include <Basic/Stack.h>
#include "Basic/Debug.h"
#include <llvm/Support/Process.h>
#include <llvm/Support/Host.h>
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/TargetSelect.h"
#include <llvm/Support/Signals.h>

using namespace fly;
using namespace llvm;

static void LLVMErrorHandler(void *UserData, const std::string &Message, bool GenCrashDiag) {
    DiagnosticsEngine &Diags = *static_cast<DiagnosticsEngine*>(UserData);

    Diags.Report(diag::err_fe_error_backend) << Message;

    // Run the interrupt handlers to make sure any special cleanups get done, in
    // particular that we remove files registered with RemoveFileOnSignal.
    llvm::sys::RunInterruptHandlers();

    // We cannot recover from llvm errors.  When reporting a fatal error, exit
    // with status 70 to generate crash diagnostics.  For BSD systems this is
    // defined as an internal software error.  Otherwise, exit with status 1.
    llvm::sys::Process::Exit(GenCrashDiag ? 70 : 1);
}

int main(int Argc, const char **Argv)
{
    noteBottomOfStack();
    llvm::InitLLVM X(Argc, Argv);
    llvm::setBugReportMsg("PLEASE submit a bug report to " FLY_BUG_REPORT_URL
            " and include the crash backtrace, preprocessed "
            "source, and associated run script.\n");
    SmallVector<const char *, 256> Args(Argv, Argv + Argc);

    if (llvm::sys::Process::FixupStandardFileDescriptors())
        return 1;

    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmPrinters();

    llvm::BumpPtrAllocator A;
    llvm::StringSaver Saver(A);

    Driver TheDriver(Args);
    CompilerInstance &CI = TheDriver.BuildCompilerInstance();
    if (!TheDriver.Execute()) {
        llvm::errs() << "Fly Compiler Error\n";
    }

    // Set an error handler, so that any LLVM backend diagnostics go through our error handler.
    llvm::install_fatal_error_handler(LLVMErrorHandler, static_cast<void*>(&CI.getDiagnostics()));

    // Shutdown after execution
    llvm::llvm_shutdown();
    return 0;
}
