//===----------------------------------------------------------------------===//
// Fly.cpp - Main
//
// Part of the Fly Project, under the Apache License v2.0
// See https://flylang.org/LICENSE.txt for license information.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//

#include "Compiler/Compiler.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/ManagedStatic.h"

using namespace fly;
using namespace llvm;

llvm::ExitOnError ExitOnErr;

int main(int argc, const char **argv)
{

    const Compiler &compiler = Compiler(argc, argv);
    bool res = compiler.execute();

    // Shutdown.
    llvm::llvm_shutdown();

    return res;
}
