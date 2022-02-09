//===--------------------------------------------------------------------------------------------------------------===//
// src/Driver/ToolChain.cpp - ToolChain
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include <llvm/Option/ArgList.h>
#include "Frontend/FrontendOptions.h"
#include "Driver/ToolChain.h"
#include "Basic/Archiver.h"
#include "Config/Config.h"
#include "Basic/Debug.h"

#include "lld/Common/Driver.h"
#include "lld/Core/Writer.h"

using namespace fly;

ToolChain::ToolChain(DiagnosticsEngine &Diag, const llvm::Triple &T, CodeGenOptions &CodeGenOpts) :
    Diag(Diag), T(T), CodeGenOpts(CodeGenOpts) {
    VFS = this->VFS = llvm::vfs::getRealFileSystem();

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
    const std::string OutFileName = FrontendOpts.getOutputFile();

    // Select right options format by platform (Win or others)
    FLY_DEBUG_MESSAGE("ToolChain", "Link", "Output=" << OutFileName);
    if (FrontendOpts.LibraryGen) {
        Archiver *Archive = new Archiver(Diag, OutFileName + ".lib");
        return Archive->CreateLib(InFiles);
    } else {
        if (T.isWindowsMSVCEnvironment()) {
            return LinkWindows(InFiles, OutFileName, Args);
        } else if (T.isOSDarwin()) {
            return LinkDarwin(InFiles, OutFileName, Args);
        } else {
            return LinkLinux(InFiles, OutFileName);
        }
    }

    assert(0 && "Unknown Object Format");
}

bool ToolChain::LinkWindows(const llvm::SmallVector<std::string, 4> &InFiles, const std::string &OutFile,
                            SmallVector<const char *, 4> &Args) {
    std::string Out = "/out:" + OutFile + ".exe";
    // Args.push_back("/entry:main");
    // https://docs.microsoft.com/en-us/cpp/c-runtime-library/crt-library-features?view=msvc-170
    Args.push_back("/defaultlib:libcmt"); // DLL import library for the UCRT.
    Args.push_back(Out.c_str());

    if (T.getObjectFormat() == llvm::Triple::COFF) {
        return lld::coff::link(Args, false, llvm::outs(), llvm::errs());
    }

//    assert(0 && "Invalid Object Format for Windows OS");
    return false;
}

bool ToolChain::LinkDarwin(const llvm::SmallVector<std::string, 4> &InFiles, const std::string &OutFile,
                           SmallVector<const char *, 4> &Args) {
    Args.push_back("-e");
    Args.push_back("_main");
    Args.push_back("-o");
    Args.push_back(OutFile.c_str());

    if (T.getObjectFormat() == llvm::Triple::MachO) {
        return lld::macho::link(Args, false, llvm::outs(), llvm::errs());
    }

//    assert(0 && "Invalid Object Format for Apple OS");
    return false;
}

bool ToolChain::LinkLinux(const llvm::SmallVector<std::string, 4> &InFiles, const std::string &OutFile) {
    llvm::SmallVector<std::string, 16> CmdArgs;
//    switch (T.getArch()) {
//        default:
//            break;
//        case llvm::Triple::x86:
//        case llvm::Triple::ppc:
//            CmdArgs.push_back("-m32");
//            break;
//        case llvm::Triple::x86_64:
//        case llvm::Triple::ppc64:
//        case llvm::Triple::ppc64le:
//            CmdArgs.push_back("-m64");
//            break;
//        case llvm::Triple::sparcel:
//            CmdArgs.push_back("-EL");
//            break;
//    }

    const llvm::Triple::ArchType Arch = T.getArch();
    const bool isAndroid = T.isAndroid();
    const bool IsIAMCU = T.isOSIAMCU();
    const bool IsVE = T.isVE();
    const bool IsPIE = getPIE();
    const bool IsStaticPIE = CodeGenOpts.StaticPIE;
    const bool IsStatic = CodeGenOpts.Static && !CodeGenOpts.StaticPIE;
    const bool HasCRTBeginEndFiles = T.hasEnvironment() || (T.getVendor() != llvm::Triple::MipsTechnologies);

    if (IsPIE)
        CmdArgs.push_back("-pie");

    if (IsStaticPIE) {
        CmdArgs.push_back("-static");
        CmdArgs.push_back("-pie");
        CmdArgs.push_back("--no-dynamic-linker");
        CmdArgs.push_back("-z");
        CmdArgs.push_back("text");
    }

    if (CodeGenOpts.RDynamic)
        CmdArgs.push_back("-export-dynamic");

    // ARM
    if (T.isARM() || T.isThumb() || T.isAArch64()) {
        bool IsBigEndian = isArmBigEndian();
        if (IsBigEndian)
            CmdArgs.push_back("--be8");
        IsBigEndian = IsBigEndian || Arch == llvm::Triple::aarch64_be;
        CmdArgs.push_back(IsBigEndian ? "-EB" : "-EL");
    }

    // Most Android ARM64 targets should enable the linker fix for erratum
    // 843419. Only non-Cortex-A53 devices are allowed to skip this flag.
//    if (Arch == llvm::Triple::aarch64 && isAndroid) {
//        std::string CPU = getCPUName(Args, T);
//        if (CPU.empty() || CPU == "generic" || CPU == "cortex-a53")
//            CmdArgs.push_back("--fix-cortex-a53-843419");
//    }

    // Android does not allow shared text relocations. Emit a warning if the
    // user's code contains any.
    if (T.isAndroid())
        CmdArgs.push_back("--warn-shared-textrel");

    CmdArgs.push_back("--eh-frame-hdr");

    if (const char *LDMOption = getLDMOption()) {
        CmdArgs.push_back("-m");
        CmdArgs.push_back(LDMOption);
    } else {
        Diag.Report(diag::err_target_unknown_triple) << T.getTriple();
        return false;
    }

    if (IsStatic) {
        if (Arch == llvm::Triple::arm || Arch == llvm::Triple::armeb ||
            Arch == llvm::Triple::thumb || Arch == llvm::Triple::thumbeb)
            CmdArgs.push_back("-Bstatic");
        else
            CmdArgs.push_back("-static");
    } else if (CodeGenOpts.Shared) {
        CmdArgs.push_back("-shared");
    }

    if (!IsStatic) {
        if (CodeGenOpts.RDynamic)
            CmdArgs.push_back("-export-dynamic");

//        if (!CodeGenOpts.Shared && !IsStaticPIE) {
//            CmdArgs.push_back("-dynamic-linker");
//            CmdArgs.push_back(getDynamicLinker(Args)));
//        }
    }

    // Output
    CmdArgs.push_back("-o");
    CmdArgs.push_back(OutFile.c_str());

    // Create Path List
    SmallVector<std::string, 16> PathList = CreatePathList();;

    // Add crt1 object
    if (!isAndroid && !IsIAMCU) {
        const char *crt1 = nullptr;
        if (!CodeGenOpts.Shared) {
            if (IsPIE)
                crt1 = "Scrt1.o";
            else if (IsStaticPIE)
                crt1 = "rcrt1.o";
            else
                crt1 = "crt1.o";
        }
        if (crt1) {
            CmdArgs.push_back(GetFilePath(crt1, PathList));
        }
        CmdArgs.push_back(GetFilePath("crti.o", PathList));
    }

    if (IsVE) {
        CmdArgs.push_back("-z");
        CmdArgs.push_back("max-page-size=0x4000000");
    }

    // Add crtbegin object
    if (IsIAMCU) {
        const std::string crt0 = GetFilePath("crt0.o", PathList);
        if (getVFS().exists(crt0))
            CmdArgs.push_back(crt0);
    } else if (HasCRTBeginEndFiles) {
        std::string P;
        if (!isAndroid) {
            std::string crtbegin = getCompilerRT("crtbegin.o", PathList);
            if (getVFS().exists(crtbegin))
                P = crtbegin;
        }
        if (P.empty()) {
            const char *crtbegin;
            if (IsStatic)
                crtbegin = isAndroid ? "crtbegin_static.o" : "crtbeginT.o";
            else if (CodeGenOpts.Shared)
                crtbegin = isAndroid ? "crtbegin_so.o" : "crtbeginS.o";
            else if (IsPIE || IsStaticPIE)
                crtbegin = isAndroid ? "crtbegin_dynamic.o" : "crtbeginS.o";
            else
                crtbegin = isAndroid ? "crtbegin_dynamic.o" : "crtbegin.o";
            P = GetFilePath(crtbegin, PathList);
        }
        CmdArgs.push_back(P);
    }

    // Add crtfastmath.o if available and fast math is enabled.
    const std::string &FastMathPath = GetFilePath("crtfastmath.o", PathList);
    if (!FastMathPath.empty()) {
        CmdArgs.push_back(FastMathPath);
    }

    // Add Inputs
    for(const std::string &ObjFile : InFiles) {
        FLY_DEBUG_MESSAGE("ToolChain", "Link", "Input=" << ObjFile);
        CmdArgs.push_back(ObjFile);
    }

    // Add Library Paths
    for(const std::string &Path : PathList) {
        FLY_DEBUG_MESSAGE("ToolChain", "Link", "LibPath=" << Path);
        CmdArgs.push_back(("-L" + Path));
    }

    if (IsStatic || IsStaticPIE)
        CmdArgs.push_back("--start-group");

    CmdArgs.push_back("-lc");
    CmdArgs.push_back("-lgcc");
    CmdArgs.push_back("-lgcc_eh");

    // Add IAMCU specific libs, if needed.
    if (IsIAMCU)
        CmdArgs.push_back("-lgloss");

    if (IsStatic || IsStaticPIE)
        CmdArgs.push_back("--end-group");

    // Add IAMCU specific libs (outside the group), if needed.
    if (IsIAMCU) {
        CmdArgs.push_back("--as-needed");
        CmdArgs.push_back("-lsoftfp");
        CmdArgs.push_back("--no-as-needed");
    }

    // Add crtend object
    if (!IsIAMCU) {
        if (HasCRTBeginEndFiles) {
            std::string P;
            if (!isAndroid) {
                std::string crtend = getCompilerRT("crtend.o", PathList);
                if (getVFS().exists(crtend))
                    P = crtend;
            }
            if (P.empty()) {
                std::string crtend;
                if (CodeGenOpts.Shared)
                    crtend = isAndroid ? "crtend_so.o" : "crtendS.o";
                else if (IsPIE || IsStaticPIE)
                    crtend = isAndroid ? "crtend_android.o" : "crtendS.o";
                else
                    crtend = isAndroid ? "crtend_android.o" : "crtend.o";
                P = GetFilePath(crtend, PathList);
            }
            CmdArgs.push_back(P);
        }
        if (!isAndroid) {
            const std::string P = GetFilePath("crtn.o", PathList);
            CmdArgs.push_back(P);
        }
    }

//    CmdArgs.push_back("-static"); // for static link
////    Args.push_back("-dynamic-linker"); // for dynamic link
////    Args.push_back("/lib64/ld-linux-x86-64.so.2"); // for dynamic link
//    CmdArgs.push_back(GCC_LIB_PATH + "/crt1.o");
//    CmdArgs.push_back("/usr/lib/x86_64-linux-gnu/crti.o");
////    Args.push_back("/usr/lib/gcc/x86_64-linux-gnu/10/crtbegin.o"); // for dynamic link
//    CmdArgs.push_back("/usr/lib/gcc/x86_64-linux-gnu/10/crtbeginT.o"); // for static link
//    for(const std::string &ObjFile : InFiles) {
//        FLY_DEBUG_MESSAGE("ToolChain", "Link", "Input=" << ObjFile);
//        CmdArgs.push_back(ObjFile.c_str());
//    }
//    CmdArgs.push_back("-L/lib");
//    CmdArgs.push_back("-L/lib64");
//    CmdArgs.push_back("-L/lib/x86_64-linux-gnu");
//    CmdArgs.push_back("-L/usr/lib/x86_64-linux-gnu");
//    std::string GccPath = "-L" + GCC_LIB_PATH;
//    CmdArgs.push_back(GccPath.c_str()); // for static link
//    CmdArgs.push_back("-lc");
//    CmdArgs.push_back("-lgcc"); // for static link
//    CmdArgs.push_back("-lgcc_eh"); // for static link
//
//    CmdArgs.push_back("/usr/lib/gcc/x86_64-linux-gnu/10/crtend.o");
//    CmdArgs.push_back("/usr/lib/x86_64-linux-gnu/crtn.o");

    // Log arguments
    std::string ArgStr;
    for (auto &A : CmdArgs) {
        ArgStr.append(A).append(" ");
    }
    FLY_DEBUG_MESSAGE("ToolChain", "Link", "Arguments: " + ArgStr);

    if (T.getObjectFormat() == llvm::Triple::ELF) {
        SmallVector<const char*, 16> LinkArgs;
        for (auto &Arg : CmdArgs) {
            LinkArgs.push_back(Arg.c_str());
        }
        return lld::elf::link(LinkArgs, false, llvm::outs(), llvm::errs());
    }

//    assert(Diag"Invalid Object Format for Linux OS");
    return false;
}

bool ToolChain::getPIE() {
    if (CodeGenOpts.Shared || CodeGenOpts.Static)
        return false;
    return CodeGenOpts.PIE;
}

bool ToolChain::isArmBigEndian() {
    return T.getArch() == llvm::Triple::armeb || T.getArch() == llvm::Triple::thumbeb;
}

const char *ToolChain::getLDMOption() {
    switch (T.getArch()) {
        case llvm::Triple::x86:
            if (T.isOSIAMCU())
                return "elf_iamcu";
            return "elf_i386";
        case llvm::Triple::aarch64:
            return "aarch64linux";
        case llvm::Triple::aarch64_be:
            return "aarch64linuxb";
        case llvm::Triple::arm:
        case llvm::Triple::thumb:
        case llvm::Triple::armeb:
        case llvm::Triple::thumbeb:
            return isArmBigEndian() ? "armelfb_linux_eabi" : "armelf_linux_eabi";
        case llvm::Triple::ppc:
            return "elf32ppclinux";
        case llvm::Triple::ppc64:
            return "elf64ppc";
        case llvm::Triple::ppc64le:
            return "elf64lppc";
        case llvm::Triple::riscv32:
            return "elf32lriscv";
        case llvm::Triple::riscv64:
            return "elf64lriscv";
        case llvm::Triple::sparc:
        case llvm::Triple::sparcel:
            return "elf32_sparc";
        case llvm::Triple::sparcv9:
            return "elf64_sparc";
        case llvm::Triple::mips:
            return "elf32btsmip";
        case llvm::Triple::mipsel:
            return "elf32ltsmip";
        case llvm::Triple::mips64:
//            if (tools::mips::hasMipsAbiArg(Args, "n32") ||
//                T.getEnvironment() == llvm::Triple::GNUABIN32)
//                return "elf32btsmipn32";
            return "elf64btsmip";
        case llvm::Triple::mips64el:
//            if (tools::mips::hasMipsAbiArg(Args, "n32") ||
//                T.getEnvironment() == llvm::Triple::GNUABIN32)
//                return "elf32ltsmipn32";
            return "elf64ltsmip";
        case llvm::Triple::systemz:
            return "elf64_s390";
        case llvm::Triple::x86_64:
            if (T.getEnvironment() == llvm::Triple::GNUX32)
                return "elf32_x86_64";
            return "elf_x86_64";
        case llvm::Triple::ve:
            return "elf64ve";
        default:
            return nullptr;
    }
}

std::string ToolChain::GetFilePath(llvm::Twine Name, SmallVector<std::string, 16> &PathList) const {
    // Search for Name in a list of paths.
    auto SearchPaths = [&](const llvm::SmallVectorImpl<std::string> &P)
            -> llvm::Optional<std::string> {
        // Respect a limited subset of the '-Bprefix' functionality in GCC by
        // attempting to use this prefix when looking for file paths.
        for (const auto &Dir : P) {
            if (Dir.empty())
                continue;
            SmallString<128> Path(Dir);
            llvm::sys::path::append(Path, Name);
            if (llvm::sys::fs::exists(Twine(Path))) {
                FLY_DEBUG_MESSAGE("ToolChain", "GetFilePath", "Path exists " << Path);
                return std::string(Path);
            }
        }
        return None;
    };

    if (auto P = SearchPaths(PathList))
        return *P;

    return "";
}

vfs::FileSystem &ToolChain::getVFS() const {
    return *VFS;
}

std::string ToolChain::getCompilerRT(const char *Name, SmallVector<std::string, 16> &PathList) {
    for (const auto &LibPath : PathList) {
        SmallString<128> P(LibPath);
        llvm::sys::path::append(P, Name);
        if (getVFS().exists(P))
            return std::string(P.str());
    }
    return "/lib";
}

/// Get our best guess at the multiarch triple for a target.
///
/// Debian-based systems are starting to use a multiarch setup where they use
/// a target-triple directory in the library and header search paths.
/// Unfortunately, this triple does not align with the vanilla target triple,
/// so we provide a rough mapping here.
std::string ToolChain::getMultiarch() const {
    llvm::Triple::EnvironmentType TargetEnvironment = T.getEnvironment();
    bool IsAndroid = T.isAndroid();
    bool IsMipsR6 = T.getSubArch() == llvm::Triple::MipsSubArch_r6;
    bool IsMipsN32Abi = T.getEnvironment() == llvm::Triple::GNUABIN32;

    // For most architectures, just use whatever we have rather than trying to be
    // clever.
    switch (T.getArch()) {
        default:
            break;

            // We use the existence of '/lib/<triple>' as a directory to detect some
            // common linux triples that don't quite match the Clang triple for both
            // 32-bit and 64-bit targets. Multiarch fixes its install triples to these
            // regardless of what the actual target triple is.
        case llvm::Triple::arm:
        case llvm::Triple::thumb:
            if (IsAndroid) {
                return "arm-linux-androideabi";
            } else if (TargetEnvironment == llvm::Triple::GNUEABIHF) {
                if (getVFS().exists("/lib/arm-linux-gnueabihf"))
                    return "arm-linux-gnueabihf";
            } else {
                if (getVFS().exists("/lib/arm-linux-gnueabi"))
                    return "arm-linux-gnueabi";
            }
            break;
        case llvm::Triple::armeb:
        case llvm::Triple::thumbeb:
            if (TargetEnvironment == llvm::Triple::GNUEABIHF) {
                if (getVFS().exists("/lib/armeb-linux-gnueabihf"))
                    return "armeb-linux-gnueabihf";
            } else {
                if (getVFS().exists("/lib/armeb-linux-gnueabi"))
                    return "armeb-linux-gnueabi";
            }
            break;
        case llvm::Triple::x86:
            if (IsAndroid)
                return "i686-linux-android";
            if (getVFS().exists("/lib/i386-linux-gnu"))
                return "i386-linux-gnu";
            break;
        case llvm::Triple::x86_64:
            if (IsAndroid)
                return "x86_64-linux-android";
            // We don't want this for x32, otherwise it will match x86_64 libs
            if (TargetEnvironment != llvm::Triple::GNUX32 &&
                getVFS().exists("/lib/x86_64-linux-gnu"))
                return "x86_64-linux-gnu";
            break;
        case llvm::Triple::aarch64:
            if (IsAndroid)
                return "aarch64-linux-android";
            if (getVFS().exists("/lib/aarch64-linux-gnu"))
                return "aarch64-linux-gnu";
            break;
        case llvm::Triple::aarch64_be:
            if (getVFS().exists("/lib/aarch64_be-linux-gnu"))
                return "aarch64_be-linux-gnu";
            break;
        case llvm::Triple::mips: {
            std::string MT = IsMipsR6 ? "mipsisa32r6-linux-gnu" : "mips-linux-gnu";
            if (getVFS().exists("/lib/" + MT))
                return MT;
            break;
        }
        case llvm::Triple::mipsel: {
            if (IsAndroid)
                return "mipsel-linux-android";
            std::string MT = IsMipsR6 ? "mipsisa32r6el-linux-gnu" : "mipsel-linux-gnu";
            if (getVFS().exists("/lib/" + MT))
                return MT;
            break;
        }
        case llvm::Triple::mips64: {
            std::string MT = std::string(IsMipsR6 ? "mipsisa64r6" : "mips64") +
                             "-linux-" + (IsMipsN32Abi ? "gnuabin32" : "gnuabi64");
            if (getVFS().exists("/lib/" + MT))
                return MT;
            if (getVFS().exists("/lib/mips64-linux-gnu"))
                return "mips64-linux-gnu";
            break;
        }
        case llvm::Triple::mips64el: {
            if (IsAndroid)
                return "mips64el-linux-android";
            std::string MT = std::string(IsMipsR6 ? "mipsisa64r6el" : "mips64el") +
                             "-linux-" + (IsMipsN32Abi ? "gnuabin32" : "gnuabi64");
            if (getVFS().exists("/lib/" + MT))
                return MT;
            if (getVFS().exists("/lib/mips64el-linux-gnu"))
                return "mips64el-linux-gnu";
            break;
        }
        case llvm::Triple::ppc:
            if (getVFS().exists("/lib/powerpc-linux-gnuspe"))
                return "powerpc-linux-gnuspe";
            if (getVFS().exists("/lib/powerpc-linux-gnu"))
                return "powerpc-linux-gnu";
            break;
        case llvm::Triple::ppc64:
            if (getVFS().exists("/lib/powerpc64-linux-gnu"))
                return "powerpc64-linux-gnu";
            break;
        case llvm::Triple::ppc64le:
            if (getVFS().exists("/lib/powerpc64le-linux-gnu"))
                return "powerpc64le-linux-gnu";
            break;
        case llvm::Triple::sparc:
            if (getVFS().exists("/lib/sparc-linux-gnu"))
                return "sparc-linux-gnu";
            break;
        case llvm::Triple::sparcv9:
            if (getVFS().exists("/lib/sparc64-linux-gnu"))
                return "sparc64-linux-gnu";
            break;
        case llvm::Triple::systemz:
            if (getVFS().exists("/lib/s390x-linux-gnu"))
                return "s390x-linux-gnu";
            break;
    }
    return T.str();
}

std::string ToolChain::getOSLibDir() {
    // FIXME
//    if (Triple.isMIPS()) {
//        if (Triple.isAndroid()) {
//            StringRef CPUName;
//            StringRef ABIName;
//            tools::mips::getMipsCPUAndABI(Args, Triple, CPUName, ABIName);
//            if (CPUName == "mips32r6")
//                return "libr6";
//            if (CPUName == "mips32r2")
//                return "libr2";
//        }
//        // lib32 directory has a special meaning on MIPS targets.
//        // It contains N32 ABI binaries. Use this folder if produce
//        // code for N32 ABI only.
//        if (tools::mips::hasMipsAbiArg(Args, "n32"))
//            return "lib32";
//        return Triple.isArch32Bit() ? "lib" : "lib64";
//    }

    // It happens that only x86 and PPC use the 'lib32' variant of oslibdir, and
    // using that variant while targeting other architectures causes problems
    // because the libraries are laid out in shared system roots that can't cope
    // with a 'lib32' library search path being considered. So we only enable
    // them when we know we may need it.
    //
    // FIXME: This is a bit of a hack. We should really unify this code for
    // reasoning about oslibdir spellings with the lib dir spellings in the
    // GCCInstallationDetector, but that is a more significant refactoring.
    if (T.getArch() == llvm::Triple::x86 ||
        T.getArch() == llvm::Triple::ppc)
        return "lib32";

    if (T.getArch() == llvm::Triple::x86_64 &&
        T.getEnvironment() == llvm::Triple::GNUX32)
        return "libx32";

    if (T.getArch() == llvm::Triple::riscv32)
        return "lib32";

    return T.isArch32Bit() ? "lib" : "lib64";
}

SmallVector<std::string, 16> ToolChain::CreatePathList() {
    SmallVector<std::string, 16> PathList;
    const std::string &MultiarchTriple = getMultiarch();
    const std::string &OSLibDir = getOSLibDir();

    // Examples: /lib32 /lib64 /lib ...
    SmallString<128> OSLib("/" + getOSLibDir());
    if (getVFS().exists(OSLib))
        PathList.push_back(OSLib.str().str());

    // /lib
    if (OSLib != "/lib") {
        SmallString<128> Lib("/lib");
        if (getVFS().exists(Lib))
            PathList.push_back(Lib.str().str());
    }

    // /usr/lib
    SmallString<128> UsrLib("/usr/lib");
    if (getVFS().exists(UsrLib))
        PathList.push_back(UsrLib.str().str());

    // Example: /lib/x86_64-linux-gnu
    SmallString<128> LibArch("/lib");
    llvm::sys::path::append(LibArch, MultiarchTriple);
    if (getVFS().exists(LibArch))
        PathList.push_back(LibArch.str().str());

    // Example: /usr/lib/x86_64-linux-gnu
    SmallString<128> UsrLibArch("/usr/lib");
    llvm::sys::path::append(UsrLibArch, MultiarchTriple);
    if (getVFS().exists(UsrLibArch))
        PathList.push_back(UsrLibArch.str().str());

    // Example: /lib/gcc/x86_64-linux-gnu/10
    if (getVFS().exists(GCC_LIB_PATH))
        PathList.push_back(GCC_LIB_PATH);
    return PathList;
}
