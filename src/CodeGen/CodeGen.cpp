//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGen.cpp - Code Generator implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGen.h"
#include "Basic/FileManager.h"
#include <llvm/Target/TargetMachine.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <CodeGen/BackendUtil.h>
#include <llvm/IR/IRPrintingPasses.h>

namespace llvm {
    class Target;

    namespace sys {

        namespace fs {}
    }
}

namespace llvm {  };

using namespace fly;

CodeGen::CodeGen(DiagnosticsEngine &diags, ASTContext &context, TargetInfo &target) :
        diags(diags), context(context), builder(new CodeGenModule(diags, context.getFileName())), target(target) {

}

bool CodeGen::execute() const {
    builder->initialize(target);

    std::error_code code;

    // FIXME take from options
    BackendAction backendAction = Backend_EmitLL;
    std::unique_ptr<llvm::raw_fd_ostream> os;

//    llvm::PassBuilder passBuilder;
//    ModuleAnalysisManager MAM;
//    passBuilder.registerModuleAnalyses(MAM);
//    llvm::ModulePassManager modulePassManager;
    switch (backendAction) {
        case Backend_EmitNothing:
            break;
        case Backend_EmitLL:
            os = make_unique<raw_fd_ostream>(context.getFileName().str() + ".o", code, llvm::sys::fs::F_None);

            break;
        case Backend_EmitBC:
            break;
        case Backend_EmitAssembly:
            break;
        case Backend_EmitObj:
            break;
    }

//    unique_ptr<ModulePass> printModPass (createPrintModulePass(*os, ""));
////
////    MAM.registerPass(printModPass);
//    modulePassManager.addPass(&printModPass);
//
//    modulePassManager.run(*builder->module, MAM);

    ModulePassManager MPM;
    PassBuilder PB;
    LoopAnalysisManager LAM;
    FunctionAnalysisManager FAM;
    CGSCCAnalysisManager CGAM;
    ModuleAnalysisManager MAM;

    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    MPM.run(*builder->module, MAM);

    os->flush();
    return true;
}
