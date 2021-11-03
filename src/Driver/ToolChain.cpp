//===--------------------------------------------------------------------------------------------------------------===//
// src/Driver/ToolChain.cpp - ToolChain
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Driver/ToolChain.h"
#include "lld/Common/Driver.h"
#include "lld/Core/Writer.h"
#include "Basic/Debug.h"

using namespace fly;

ToolChain::ToolChain(const llvm::Triple &T) : T(T){

}

bool ToolChain::Link(const llvm::SmallVector<std::string, 4> &ObjFiles, llvm::StringRef OutFile) {
    llvm::SmallVector<const char *, 4> Args = {"fly"};
    for(const std::string &ObjFile : ObjFiles) {
        FLY_DEBUG_MESSAGE("ToolChain", "Link", "Input=" << ObjFile);
        Args.push_back(ObjFile.c_str());
    }
    Args.push_back("-o");
    FLY_DEBUG_MESSAGE("ToolChain", "Link", "Output=" << OutFile);
    Args.push_back(OutFile.str().c_str());

    switch (T.getObjectFormat()) {
        case llvm::Triple::MachO:
            lld::macho::link(Args, true, llvm::outs(), llvm::errs());
            break;
        case llvm::Triple::COFF:
            lld::coff::link(Args, true, llvm::outs(), llvm::errs());
            break;
        case llvm::Triple::ELF:
            lld::elf::link(Args, true, llvm::outs(), llvm::errs());
            break;
        case llvm::Triple::Wasm:
            lld::wasm::link(Args, true, llvm::outs(), llvm::errs());
            break;
    }
    assert(0 && "Unknown Object Format");
    return false;
}
