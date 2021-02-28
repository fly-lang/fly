//===----------------------------------------------------------------------===//
// src/Frontend/Frontend.cpp - Main Compiler Frontend
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//
#include "Frontend/Frontend.h"

using namespace fly;

Frontend::Frontend(CompilerInvocation &invocation) : invocation(invocation), diagnostics(invocation.getDiagnostics()) {

    // Create Compiler Instance for each input file
    for (const auto &InputFile : invocation.getFrontendOptions().getInputFiles()) {
        instances.push_back(new CompilerInstance(invocation, InputFile));
    }
}

Frontend::~Frontend() {
    //assert(OutputFiles.empty() && "Still output files in flight?");
    // TODO delete CompilerInvocation
    // Delete Compiler Instances
}

bool Frontend::execute() const {
    bool res = true;
    for (auto &instance : instances) {
        res &= instance->execute();
    }
    return res;
}

CompilerInvocation &Frontend::getInvocation() {
    return invocation;
}

const std::vector<CompilerInstance *> &Frontend::getInstances() const {
    return instances;
}
