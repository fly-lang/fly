//===--- Targets.cpp - Implement target feature support -------------------===//
//
// Part of the Fly Project, under the Apache License v2.0
//
//===----------------------------------------------------------------------===//
//
// This file implements construction of a TargetInfo object from a
// target triple.
//
//===----------------------------------------------------------------------===//

#include "Basic/Targets/Targets.h"

#include "Basic/Targets/AArch64.h"
#include "Basic/Targets/AMDGPU.h"
#include "Basic/Targets/ARC.h"
#include "Basic/Targets/ARM.h"
#include "Basic/Targets/AVR.h"
#include "Basic/Targets/BPF.h"
#include "Basic/Targets/Hexagon.h"
#include "Basic/Targets/Lanai.h"
#include "Basic/Targets/Le64.h"
#include "Basic/Targets/MSP430.h"
#include "Basic/Targets/Mips.h"
#include "Basic/Targets/NVPTX.h"
#include "Basic/Targets/OSTargets.h"
#include "Basic/Targets/PNaCl.h"
#include "Basic/Targets/PPC.h"
#include "Basic/Targets/RISCV.h"
#include "Basic/Targets/SPIR.h"
#include "Basic/Targets/Sparc.h"
#include "Basic/Targets/SystemZ.h"
#include "Basic/Targets/TCE.h"
#include "Basic/Targets/WebAssembly.h"
#include "Basic/Targets/X86.h"
#include "Basic/Targets/XCore.h"
#include "Basic/Diagnostic.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/Triple.h"

using namespace fly;

namespace fly {
namespace targets {

//===----------------------------------------------------------------------===//
// Driver code
//===----------------------------------------------------------------------===//

TargetInfo *AllocateTarget(const llvm::Triple &Triple,
                           const TargetOptions &Opts) {
  llvm::Triple::OSType os = Triple.getOS();

  switch (Triple.getArch()) {
  default:
    return nullptr;

  case llvm::Triple::arc:
    return new ARCTargetInfo(Triple, Opts);

  case llvm::Triple::xcore:
    return new XCoreTargetInfo(Triple, Opts);

  case llvm::Triple::hexagon:
    return new HexagonTargetInfo(Triple, Opts);

  case llvm::Triple::lanai:
    return new LanaiTargetInfo(Triple, Opts);

  case llvm::Triple::aarch64_32:
    if (Triple.isOSDarwin())
      return new DarwinAArch64TargetInfo(Triple, Opts);

    return nullptr;
  case llvm::Triple::aarch64:
    if (Triple.isOSDarwin())
      return new DarwinAArch64TargetInfo(Triple, Opts);

    switch (os) {
    case llvm::Triple::CloudABI:
      return new CloudABITargetInfo<AArch64leTargetInfo>(Triple, Opts);
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<AArch64leTargetInfo>(Triple, Opts);
    case llvm::Triple::Fuchsia:
      return new FuchsiaTargetInfo<AArch64leTargetInfo>(Triple, Opts);
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<AArch64leTargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<AArch64leTargetInfo>(Triple, Opts);
    case llvm::Triple::OpenBSD:
      return new OpenBSDTargetInfo<AArch64leTargetInfo>(Triple, Opts);
    case llvm::Triple::Win32:
      switch (Triple.getEnvironment()) {
      case llvm::Triple::GNU:
        return new MinGWARM64TargetInfo(Triple, Opts);
      case llvm::Triple::MSVC:
      default: // Assume MSVC for unknown environments
        return new MicrosoftARM64TargetInfo(Triple, Opts);
      }
    default:
      return new AArch64leTargetInfo(Triple, Opts);
    }

  case llvm::Triple::aarch64_be:
    switch (os) {
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<AArch64beTargetInfo>(Triple, Opts);
    case llvm::Triple::Fuchsia:
      return new FuchsiaTargetInfo<AArch64beTargetInfo>(Triple, Opts);
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<AArch64beTargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<AArch64beTargetInfo>(Triple, Opts);
    default:
      return new AArch64beTargetInfo(Triple, Opts);
    }

  case llvm::Triple::arm:
  case llvm::Triple::thumb:
    if (Triple.isOSBinFormatMachO())
      return new DarwinARMTargetInfo(Triple, Opts);

    switch (os) {
    case llvm::Triple::CloudABI:
      return new CloudABITargetInfo<ARMleTargetInfo>(Triple, Opts);
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<ARMleTargetInfo>(Triple, Opts);
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<ARMleTargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<ARMleTargetInfo>(Triple, Opts);
    case llvm::Triple::OpenBSD:
      return new OpenBSDTargetInfo<ARMleTargetInfo>(Triple, Opts);
    case llvm::Triple::RTEMS:
      return new RTEMSTargetInfo<ARMleTargetInfo>(Triple, Opts);
    case llvm::Triple::NaCl:
      return new NaClTargetInfo<ARMleTargetInfo>(Triple, Opts);
    case llvm::Triple::Win32:
      switch (Triple.getEnvironment()) {
      case llvm::Triple::Cygnus:
        return new CygwinARMTargetInfo(Triple, Opts);
      case llvm::Triple::GNU:
        return new MinGWARMTargetInfo(Triple, Opts);
      case llvm::Triple::Itanium:
        return new ItaniumWindowsARMleTargetInfo(Triple, Opts);
      case llvm::Triple::MSVC:
      default: // Assume MSVC for unknown environments
        return new MicrosoftARMleTargetInfo(Triple, Opts);
      }
    default:
      return new ARMleTargetInfo(Triple, Opts);
    }

  case llvm::Triple::armeb:
  case llvm::Triple::thumbeb:
    if (Triple.isOSDarwin())
      return new DarwinARMTargetInfo(Triple, Opts);

    switch (os) {
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<ARMbeTargetInfo>(Triple, Opts);
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<ARMbeTargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<ARMbeTargetInfo>(Triple, Opts);
    case llvm::Triple::OpenBSD:
      return new OpenBSDTargetInfo<ARMbeTargetInfo>(Triple, Opts);
    case llvm::Triple::RTEMS:
      return new RTEMSTargetInfo<ARMbeTargetInfo>(Triple, Opts);
    case llvm::Triple::NaCl:
      return new NaClTargetInfo<ARMbeTargetInfo>(Triple, Opts);
    default:
      return new ARMbeTargetInfo(Triple, Opts);
    }

  case llvm::Triple::avr:
    return new AVRTargetInfo(Triple, Opts);
  case llvm::Triple::bpfeb:
  case llvm::Triple::bpfel:
    return new BPFTargetInfo(Triple, Opts);

  case llvm::Triple::msp430:
    return new MSP430TargetInfo(Triple, Opts);

  case llvm::Triple::mips:
    switch (os) {
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::RTEMS:
      return new RTEMSTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<MipsTargetInfo>(Triple, Opts);
    default:
      return new MipsTargetInfo(Triple, Opts);
    }

  case llvm::Triple::mipsel:
    switch (os) {
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::RTEMS:
      return new RTEMSTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::NaCl:
      return new NaClTargetInfo<NaClMips32TargetInfo>(Triple, Opts);
    default:
      return new MipsTargetInfo(Triple, Opts);
    }

  case llvm::Triple::mips64:
    switch (os) {
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::RTEMS:
      return new RTEMSTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::OpenBSD:
      return new OpenBSDTargetInfo<MipsTargetInfo>(Triple, Opts);
    default:
      return new MipsTargetInfo(Triple, Opts);
    }

  case llvm::Triple::mips64el:
    switch (os) {
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::RTEMS:
      return new RTEMSTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<MipsTargetInfo>(Triple, Opts);
    case llvm::Triple::OpenBSD:
      return new OpenBSDTargetInfo<MipsTargetInfo>(Triple, Opts);
    default:
      return new MipsTargetInfo(Triple, Opts);
    }

  case llvm::Triple::le32:
    switch (os) {
    case llvm::Triple::NaCl:
      return new NaClTargetInfo<PNaClTargetInfo>(Triple, Opts);
    default:
      return nullptr;
    }

  case llvm::Triple::le64:
    return new Le64TargetInfo(Triple, Opts);

  case llvm::Triple::ppc:
    if (Triple.isOSDarwin())
      return new DarwinPPC32TargetInfo(Triple, Opts);
    switch (os) {
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<PPC32TargetInfo>(Triple, Opts);
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<PPC32TargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<PPC32TargetInfo>(Triple, Opts);
    case llvm::Triple::OpenBSD:
      return new OpenBSDTargetInfo<PPC32TargetInfo>(Triple, Opts);
    case llvm::Triple::RTEMS:
      return new RTEMSTargetInfo<PPC32TargetInfo>(Triple, Opts);
    case llvm::Triple::AIX:
      return new AIXPPC32TargetInfo(Triple, Opts);
    default:
      return new PPC32TargetInfo(Triple, Opts);
    }

  case llvm::Triple::ppc64:
    if (Triple.isOSDarwin())
      return new DarwinPPC64TargetInfo(Triple, Opts);
    switch (os) {
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<PPC64TargetInfo>(Triple, Opts);
    case llvm::Triple::Lv2:
      return new PS3PPUTargetInfo<PPC64TargetInfo>(Triple, Opts);
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<PPC64TargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<PPC64TargetInfo>(Triple, Opts);
    case llvm::Triple::AIX:
      return new AIXPPC64TargetInfo(Triple, Opts);
    default:
      return new PPC64TargetInfo(Triple, Opts);
    }

  case llvm::Triple::ppc64le:
    switch (os) {
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<PPC64TargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<PPC64TargetInfo>(Triple, Opts);
    default:
      return new PPC64TargetInfo(Triple, Opts);
    }

  case llvm::Triple::nvptx:
    return new NVPTXTargetInfo(Triple, Opts, /*TargetPointerWidth=*/32);
  case llvm::Triple::nvptx64:
    return new NVPTXTargetInfo(Triple, Opts, /*TargetPointerWidth=*/64);

  case llvm::Triple::amdgcn:
  case llvm::Triple::r600:
    return new AMDGPUTargetInfo(Triple, Opts);

  case llvm::Triple::riscv32:
    // TODO: add cases for NetBSD, RTEMS once tested.
    switch (os) {
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<RISCV32TargetInfo>(Triple, Opts);
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<RISCV32TargetInfo>(Triple, Opts);
    default:
      return new RISCV32TargetInfo(Triple, Opts);
    }

  case llvm::Triple::riscv64:
    // TODO: add cases for NetBSD, RTEMS once tested.
    switch (os) {
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<RISCV64TargetInfo>(Triple, Opts);
    case llvm::Triple::Fuchsia:
      return new FuchsiaTargetInfo<RISCV64TargetInfo>(Triple, Opts);
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<RISCV64TargetInfo>(Triple, Opts);
    default:
      return new RISCV64TargetInfo(Triple, Opts);
    }

  case llvm::Triple::sparc:
    switch (os) {
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<SparcV8TargetInfo>(Triple, Opts);
    case llvm::Triple::Solaris:
      return new SolarisTargetInfo<SparcV8TargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<SparcV8TargetInfo>(Triple, Opts);
    case llvm::Triple::OpenBSD:
      return new OpenBSDTargetInfo<SparcV8TargetInfo>(Triple, Opts);
    case llvm::Triple::RTEMS:
      return new RTEMSTargetInfo<SparcV8TargetInfo>(Triple, Opts);
    default:
      return new SparcV8TargetInfo(Triple, Opts);
    }

  // The 'sparcel' architecture copies all the above cases except for Solaris.
  case llvm::Triple::sparcel:
    switch (os) {
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<SparcV8elTargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<SparcV8elTargetInfo>(Triple, Opts);
    case llvm::Triple::OpenBSD:
      return new OpenBSDTargetInfo<SparcV8elTargetInfo>(Triple, Opts);
    case llvm::Triple::RTEMS:
      return new RTEMSTargetInfo<SparcV8elTargetInfo>(Triple, Opts);
    default:
      return new SparcV8elTargetInfo(Triple, Opts);
    }

  case llvm::Triple::sparcv9:
    switch (os) {
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<SparcV9TargetInfo>(Triple, Opts);
    case llvm::Triple::Solaris:
      return new SolarisTargetInfo<SparcV9TargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<SparcV9TargetInfo>(Triple, Opts);
    case llvm::Triple::OpenBSD:
      return new OpenBSDTargetInfo<SparcV9TargetInfo>(Triple, Opts);
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<SparcV9TargetInfo>(Triple, Opts);
    default:
      return new SparcV9TargetInfo(Triple, Opts);
    }

  case llvm::Triple::systemz:
    switch (os) {
    case llvm::Triple::Linux:
      return new LinuxTargetInfo<SystemZTargetInfo>(Triple, Opts);
    default:
      return new SystemZTargetInfo(Triple, Opts);
    }

  case llvm::Triple::tce:
    return new TCETargetInfo(Triple, Opts);

  case llvm::Triple::tcele:
    return new TCELETargetInfo(Triple, Opts);

  case llvm::Triple::x86:
    if (Triple.isOSDarwin())
      return new DarwinI386TargetInfo(Triple, Opts);

    switch (os) {
    case llvm::Triple::Ananas:
      return new AnanasTargetInfo<X86_32TargetInfo>(Triple, Opts);
    case llvm::Triple::CloudABI:
      return new CloudABITargetInfo<X86_32TargetInfo>(Triple, Opts);
    case llvm::Triple::Linux: {
      switch (Triple.getEnvironment()) {
      default:
        return new LinuxTargetInfo<X86_32TargetInfo>(Triple, Opts);
      case llvm::Triple::Android:
        return new AndroidX86_32TargetInfo(Triple, Opts);
      }
    }
    case llvm::Triple::DragonFly:
      return new DragonFlyBSDTargetInfo<X86_32TargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDI386TargetInfo(Triple, Opts);
    case llvm::Triple::OpenBSD:
      return new OpenBSDI386TargetInfo(Triple, Opts);
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<X86_32TargetInfo>(Triple, Opts);
    case llvm::Triple::KFreeBSD:
      return new KFreeBSDTargetInfo<X86_32TargetInfo>(Triple, Opts);
    case llvm::Triple::Minix:
      return new MinixTargetInfo<X86_32TargetInfo>(Triple, Opts);
    case llvm::Triple::Solaris:
      return new SolarisTargetInfo<X86_32TargetInfo>(Triple, Opts);
    case llvm::Triple::Win32: {
      switch (Triple.getEnvironment()) {
      case llvm::Triple::Cygnus:
        return new CygwinX86_32TargetInfo(Triple, Opts);
      case llvm::Triple::GNU:
        return new MinGWX86_32TargetInfo(Triple, Opts);
      case llvm::Triple::Itanium:
      case llvm::Triple::MSVC:
      default: // Assume MSVC for unknown environments
        return new MicrosoftX86_32TargetInfo(Triple, Opts);
      }
    }
    case llvm::Triple::Haiku:
      return new HaikuX86_32TargetInfo(Triple, Opts);
    case llvm::Triple::RTEMS:
      return new RTEMSX86_32TargetInfo(Triple, Opts);
    case llvm::Triple::NaCl:
      return new NaClTargetInfo<X86_32TargetInfo>(Triple, Opts);
    case llvm::Triple::ELFIAMCU:
      return new MCUX86_32TargetInfo(Triple, Opts);
    case llvm::Triple::Hurd:
      return new HurdTargetInfo<X86_32TargetInfo>(Triple, Opts);
    default:
      return new X86_32TargetInfo(Triple, Opts);
    }

  case llvm::Triple::x86_64:
    if (Triple.isOSDarwin() || Triple.isOSBinFormatMachO())
      return new DarwinX86_64TargetInfo(Triple, Opts);

    switch (os) {
    case llvm::Triple::Ananas:
      return new AnanasTargetInfo<X86_64TargetInfo>(Triple, Opts);
    case llvm::Triple::CloudABI:
      return new CloudABITargetInfo<X86_64TargetInfo>(Triple, Opts);
    case llvm::Triple::Linux: {
      switch (Triple.getEnvironment()) {
      default:
        return new LinuxTargetInfo<X86_64TargetInfo>(Triple, Opts);
      case llvm::Triple::Android:
        return new AndroidX86_64TargetInfo(Triple, Opts);
      }
    }
    case llvm::Triple::DragonFly:
      return new DragonFlyBSDTargetInfo<X86_64TargetInfo>(Triple, Opts);
    case llvm::Triple::NetBSD:
      return new NetBSDTargetInfo<X86_64TargetInfo>(Triple, Opts);
    case llvm::Triple::OpenBSD:
      return new OpenBSDX86_64TargetInfo(Triple, Opts);
    case llvm::Triple::FreeBSD:
      return new FreeBSDTargetInfo<X86_64TargetInfo>(Triple, Opts);
    case llvm::Triple::Fuchsia:
      return new FuchsiaTargetInfo<X86_64TargetInfo>(Triple, Opts);
    case llvm::Triple::KFreeBSD:
      return new KFreeBSDTargetInfo<X86_64TargetInfo>(Triple, Opts);
    case llvm::Triple::Solaris:
      return new SolarisTargetInfo<X86_64TargetInfo>(Triple, Opts);
    case llvm::Triple::Win32: {
      switch (Triple.getEnvironment()) {
      case llvm::Triple::Cygnus:
        return new CygwinX86_64TargetInfo(Triple, Opts);
      case llvm::Triple::GNU:
        return new MinGWX86_64TargetInfo(Triple, Opts);
      case llvm::Triple::MSVC:
      default: // Assume MSVC for unknown environments
        return new MicrosoftX86_64TargetInfo(Triple, Opts);
      }
    }
    case llvm::Triple::Haiku:
      return new HaikuTargetInfo<X86_64TargetInfo>(Triple, Opts);
    case llvm::Triple::NaCl:
      return new NaClTargetInfo<X86_64TargetInfo>(Triple, Opts);
    case llvm::Triple::PS4:
      return new PS4OSTargetInfo<X86_64TargetInfo>(Triple, Opts);
    default:
      return new X86_64TargetInfo(Triple, Opts);
    }

  case llvm::Triple::spir: {
    if (Triple.getOS() != llvm::Triple::UnknownOS ||
        Triple.getEnvironment() != llvm::Triple::UnknownEnvironment)
      return nullptr;
    return new SPIR32TargetInfo(Triple, Opts);
  }
  case llvm::Triple::spir64: {
    if (Triple.getOS() != llvm::Triple::UnknownOS ||
        Triple.getEnvironment() != llvm::Triple::UnknownEnvironment)
      return nullptr;
    return new SPIR64TargetInfo(Triple, Opts);
  }
  case llvm::Triple::wasm32:
    if (Triple.getSubArch() != llvm::Triple::NoSubArch ||
        Triple.getVendor() != llvm::Triple::UnknownVendor ||
        !Triple.isOSBinFormatWasm())
      return nullptr;
    switch (Triple.getOS()) {
      case llvm::Triple::WASI:
        return new WASITargetInfo<WebAssembly32TargetInfo>(Triple, Opts);
      case llvm::Triple::Emscripten:
        return new EmscriptenTargetInfo<WebAssembly32TargetInfo>(Triple, Opts);
      case llvm::Triple::UnknownOS:
        return new WebAssemblyOSTargetInfo<WebAssembly32TargetInfo>(Triple, Opts);
      default:
        return nullptr;
    }
  case llvm::Triple::wasm64:
    if (Triple.getSubArch() != llvm::Triple::NoSubArch ||
        Triple.getVendor() != llvm::Triple::UnknownVendor ||
        !Triple.isOSBinFormatWasm())
      return nullptr;
    switch (Triple.getOS()) {
      case llvm::Triple::WASI:
        return new WASITargetInfo<WebAssembly64TargetInfo>(Triple, Opts);
      case llvm::Triple::Emscripten:
        return new EmscriptenTargetInfo<WebAssembly64TargetInfo>(Triple, Opts);
      case llvm::Triple::UnknownOS:
        return new WebAssemblyOSTargetInfo<WebAssembly64TargetInfo>(Triple, Opts);
      default:
        return nullptr;
    }

  case llvm::Triple::renderscript32:
    return new LinuxTargetInfo<RenderScript32TargetInfo>(Triple, Opts);
  case llvm::Triple::renderscript64:
    return new LinuxTargetInfo<RenderScript64TargetInfo>(Triple, Opts);
  }
}
} // namespace targets
} // namespace clang

using namespace fly::targets;
/// CreateTargetInfo - Return the target info object for the specified target
/// options.
TargetInfo *
TargetInfo::CreateTargetInfo(DiagnosticsEngine &Diags,
                             const std::shared_ptr<TargetOptions> &Opts) {
  llvm::Triple Triple(Opts->Triple);

  // Construct the target
  std::unique_ptr<TargetInfo> Target(AllocateTarget(Triple, *Opts));
  if (!Target) {
    Diags.Report(diag::err_target_unknown_triple) << Triple.str();
    return nullptr;
  }
  Target->TargetOpts = Opts;

  // Set the target CPU if specified.
  if (!Opts->CPU.empty() && !Target->setCPU(Opts->CPU)) {
    Diags.Report(diag::err_target_unknown_cpu) << Opts->CPU;
    SmallVector<StringRef, 32> ValidList;
    Target->fillValidCPUList(ValidList);
    if (!ValidList.empty())
      Diags.Report(diag::note_valid_options) << llvm::join(ValidList, ", ");
    return nullptr;
  }

  // Set the target ABI if specified.
  if (!Opts->ABI.empty() && !Target->setABI(Opts->ABI)) {
    Diags.Report(diag::err_target_unknown_abi) << Opts->ABI;
    return nullptr;
  }

  // Set the fp math unit.
  if (!Opts->FPMath.empty() && !Target->setFPMath(Opts->FPMath)) {
    Diags.Report(diag::err_target_unknown_fpmath) << Opts->FPMath;
    return nullptr;
  }

  // Compute the default target features, we need the target to handle this
  // because features may have dependencies on one another.
  llvm::StringMap<bool> Features;
  if (!Target->initFeatureMap(Features, Diags, Opts->CPU,
                              Opts->FeaturesAsWritten))
    return nullptr;

  // Add the features to the compile options.
  Opts->Features.clear();
  for (const auto &F : Features)
    Opts->Features.push_back((F.getValue() ? "+" : "-") + F.getKey().str());
  // Sort here, so we handle the features in a predictable order. (This matters
  // when we're dealing with features that overlap.)
  llvm::sort(Opts->Features);

  if (!Target->handleTargetFeatures(Opts->Features, Diags))
    return nullptr;

  Target->setMaxAtomicWidth();

  if (!Target->validateTarget(Diags))
    return nullptr;

  Target->CheckFixedPointBits();

  return Target.release();
}
