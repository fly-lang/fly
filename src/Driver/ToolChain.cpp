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

    // Select right options format by platform (Win or others)
    FLY_DEBUG_MESSAGE("ToolChain", "Link", "Output=" << OutFile);
    if (T.isWindowsMSVCEnvironment()) {
        Args.push_back("/entry:main");
        std::string Out = "/out:" + OutFile.str() + ".exe";
        Args.push_back(Out.c_str());
    } else if (T.isOSDarwin()) {
        Args.push_back("-e");
        Args.push_back("_main");
        Args.push_back("-o");
        Args.push_back(OutFile.str().c_str());
    } else{
        Args.push_back("--entry=main");
        Args.push_back("-o");
        Args.push_back(OutFile.str().c_str());
    }

    // Link by selecting the Object code format
    switch (T.getObjectFormat()) {
        case llvm::Triple::MachO:
            return lld::macho::link(Args, false, llvm::outs(), llvm::errs());
        case llvm::Triple::COFF:
            return lld::coff::link(Args, false, llvm::outs(), llvm::errs());
        case llvm::Triple::ELF:
            return lld::elf::link(Args, false, llvm::outs(), llvm::errs());
        case llvm::Triple::Wasm:
            return lld::wasm::link(Args, false, llvm::outs(), llvm::errs());
    }
    assert(0 && "Unknown Object Format");
}
