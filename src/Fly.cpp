//===--------------------------------------------------------------------------------------------------------------===//
// src/Fly.cpp - Main
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Driver/Driver.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/ManagedStatic.h"

using namespace fly;
using namespace llvm;

llvm::ExitOnError ExitOnErr;

int main(int argc, const char **argv)
{

    Driver driver(argc, argv);
    bool res = driver.execute();

    // Shutdown.
    llvm::llvm_shutdown();

    return res;
}
