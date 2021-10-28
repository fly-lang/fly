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

using namespace fly;

ToolChain::ToolChain(const llvm::Triple &T) : T(T){

}

bool ToolChain::Link(llvm::StringRef File) {
    llvm::ArrayRef<const char *> Args = {"fly", File.str().c_str()};

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
