//
// Created by marco on 2/24/21.
//

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