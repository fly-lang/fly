//===--------------------------------------------------------------------------------------------------------------===//
// src/Driver/ToolChain.cpp - ToolChain
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Frontend/FrontendOptions.h"
#include "Driver/ToolChain.h"
#include "Driver/Archiver.h"
#include "Config/Config.h"
#include "Basic/Debug.h"

#include "lld/Common/Driver.h"
#include "lld/Core/Writer.h"

using namespace fly;

ToolChain::ToolChain(DiagnosticsEngine &Diag, const llvm::Triple &T) : Diag(Diag), T(T){

}

/**
 * Read /lib/base
 * @return
 */
bool BuildLib() {

    return false;
}

bool ToolChain::BuildOutput(const llvm::SmallVector<std::string, 4> &InFiles, FrontendOptions &FrontendOpts) {
    llvm::SmallVector<const char *, 4> Args = {"fly"};
    const OutputFile &OutFile = FrontendOpts.getOutputFile();

    // Select right options format by platform (Win or others)
    FLY_DEBUG_MESSAGE("ToolChain", "Link", "Output=" << OutFile.getFile());
    if (T.isWindowsMSVCEnvironment()) {
        return LinkWindows(InFiles, OutFile.getFile(), Args);
    } else if (T.isOSDarwin()) {
        return LinkDarwin(InFiles, OutFile.getFile(), Args);
    } else{
        if (FrontendOpts.LibraryGen) {
            Archiver *Archive = new Archiver(Diag, OutFile.getFile() + ".lib");
            return Archive->CreateLib(InFiles);
        }
        return LinkLinux(InFiles, OutFile.getFile(), Args);
    }

    assert(0 && "Unknown Object Format");
}

bool ToolChain::LinkWindows(const llvm::SmallVector<std::string, 4> &InFiles, llvm::StringRef OutFile,
                            SmallVector<const char *, 4> &Args) {
    std::string Out = "/out:" + OutFile.str() + ".exe";
    // Args.push_back("/entry:main");
    // https://docs.microsoft.com/en-us/cpp/c-runtime-library/crt-library-features?view=msvc-170
    Args.push_back("/defaultlib:libcmt"); // DLL import library for the UCRT.
    Args.push_back(Out.c_str());

    if (T.getObjectFormat() == llvm::Triple::COFF) {
        return lld::coff::link(Args, false, llvm::outs(), llvm::errs());
    }

    assert(0 && "Invalid Object Format for Windows OS");
}

bool ToolChain::LinkDarwin(const llvm::SmallVector<std::string, 4> &InFiles, llvm::StringRef OutFile,
                           SmallVector<const char *, 4> &Args) {
    Args.push_back("-e");
    Args.push_back("_main");
    Args.push_back("-o");
    Args.push_back(OutFile.str().c_str());

    if (T.getObjectFormat() == llvm::Triple::MachO) {
        return lld::macho::link(Args, false, llvm::outs(), llvm::errs());
    }

    assert(0 && "Invalid Object Format for Apple OS");
}

bool ToolChain::LinkLinux(const llvm::SmallVector<std::string, 4> &InFiles, llvm::StringRef OutFile,
                          SmallVector<const char *, 4> &Args) {
    // Args.push_back("--entry=main");
    Args.push_back("-o");
    Args.push_back(OutFile.str().c_str());
    Args.push_back("-static"); // for static link
//    Args.push_back("-dynamic-linker"); // for dynamic link
//    Args.push_back("/lib64/ld-linux-x86-64.so.2"); // for dynamic link
    Args.push_back("/usr/lib/x86_64-linux-gnu/crt1.o");
    Args.push_back("/usr/lib/x86_64-linux-gnu/crti.o");
//    Args.push_back("/usr/lib/gcc/x86_64-linux-gnu/10/crtbegin.o"); // for dynamic link
    Args.push_back("/usr/lib/gcc/x86_64-linux-gnu/10/crtbeginT.o"); // for static link
    for(const std::string &ObjFile : InFiles) {
        FLY_DEBUG_MESSAGE("ToolChain", "Link", "Input=" << ObjFile);
        Args.push_back(ObjFile.c_str());
    }
    Args.push_back("-L/lib");
    Args.push_back("-L/lib64");
    Args.push_back("-L/lib/x86_64-linux-gnu");
    Args.push_back("-L/usr/lib/x86_64-linux-gnu");
    std::string GccPath = "-L" + GCC_LIB_PATH;
    Args.push_back(GccPath.c_str()); // for static link
    Args.push_back("-lc");
    Args.push_back("-lgcc"); // for static link
    Args.push_back("-lgcc_eh"); // for static link

    Args.push_back("/usr/lib/gcc/x86_64-linux-gnu/10/crtend.o");
    Args.push_back("/usr/lib/x86_64-linux-gnu/crtn.o");

    std::string ArgStr;
    for (auto Arg : Args) {
        ArgStr.append(Arg).append(" ");
    }
    FLY_DEBUG_MESSAGE("ToolChain", "Link", "Arguments: " + ArgStr);

    if (T.getObjectFormat() == llvm::Triple::ELF) {
        return lld::elf::link(Args, false, llvm::outs(), llvm::errs());
    }

    assert(0 && "Invalid Object Format for Linux OS");
}
