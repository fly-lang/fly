//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/BackendUtil.cpp - CodeGen Backend Util
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/BackendUtil.h"

//void BackendUtil::CreateTargetMachine() {
//    // Create the TargetMachine for generating code.
//    std::string Error;
//    std::string Triple = TheModule->getTargetTriple();
//    const llvm::Target *TheTarget = TargetRegistry::lookupTarget(Triple, Error);
//    if (!TheTarget) {
//        Diags.Report(diag::err_fe_unable_to_create_target) << Error;
//        return;
//    }
//
//    Optional<llvm::CodeModel::Model> CM = getCodeModel(CodeGenOpts);
//    std::string FeaturesStr =
//            llvm::join(TargetOpts.Features.begin(), TargetOpts.Features.end(), ",");
//    llvm::Reloc::Model RM = CodeGenOpts.RelocationModel;
//    CodeGenOpt::Level OptLevel = getCGOptLevel(CodeGenOpts);
//
//    llvm::TargetOptions Options;
//    TheTarget->createTargetMachine(Triple, TargetOpts.CPU, FeaturesStr,
//                                            Options, RM, CM, OptLevel);
//}