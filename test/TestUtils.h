//
// Created by marco on 8/19/21.
//

#ifndef FLY_TESTUTILS_H
#define FLY_TESTUTILS_H

#include "Driver/DriverOptions.h"
#include "Basic/Diagnostic.h"
#include "Basic/DiagnosticIDs.h"
#include "Basic/FileSystemOptions.h"
#include "Basic/TargetOptions.h"
#include "Frontend/Frontend.h"
#include "Frontend/FrontendOptions.h"
#include "Frontend/CompilerInstance.h"
#include "CodeGen/BackendUtil.h"
#include "llvm/Support/Host.h"

using namespace fly;

class TestUtils {

public:

    static CodeGen *CreateCodeGen(const CompilerInstance &CI) {
        CI.getTargetOptions()->Triple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
        CI.getTargetOptions()->CodeModel = "default";
        return new CodeGen(CI.getDiagnostics(), CI.getCodeGenOptions(), CI.getTargetOptions(),
                           CI.getFrontendOptions().BackendAction, CI.getFrontendOptions().ShowTimers);
    }

    static std::shared_ptr<CompilerInstance> CreateCompilerInstance() {
        IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions;
        IntrusiveRefCntPtr<DiagnosticIDs> DiagID = new DiagnosticIDs();
        TextDiagnosticPrinter *DiagClient = new TextDiagnosticPrinter(llvm::errs(), &*DiagOpts);
        IntrusiveRefCntPtr<DiagnosticsEngine> Diags = new DiagnosticsEngine(DiagID, DiagOpts,
                                                                            DiagClient);
        ProcessWarningOptions(*Diags, *DiagOpts, /*ReportDiags=*/true);


        FileSystemOptions fileSystemOpts;
        std::shared_ptr<fly::TargetOptions> targetOpts = std::make_shared<fly::TargetOptions>();
        FrontendOptions *FrontendOpts = new FrontendOptions();
        CodeGenOptions *CodeGenOpts = new CodeGenOptions();
        std::shared_ptr<CompilerInstance> CI = std::make_shared<CompilerInstance>(Diags,
                                                                                  std::move(fileSystemOpts),
                                                                                  FrontendOpts,
                                                                                  CodeGenOpts,
                                                                                  std::move(targetOpts));
        if (!CI) {
            llvm::errs() << "Error while creating compiler instance!" << "\n";
            exit(1);
        }

        return CI;
    }
};


#endif //FLY_TESTUTILS_H
