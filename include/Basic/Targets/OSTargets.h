//===--- OSTargets.h - Declare OS target feature support --------*- C++ -*-===//
//
// Part of the Fly Project, under the Apache License v2.0
//
//===----------------------------------------------------------------------===//
//
// This file declares OS specific TargetInfo types.
//===----------------------------------------------------------------------===//

#ifndef LLVM_FLY_LIB_BASIC_TARGETS_OSTARGETS_H
#define LLVM_FLY_LIB_BASIC_TARGETS_OSTARGETS_H

#include "Targets.h"
#include "llvm/MC/MCSectionMachO.h"

namespace fly {
namespace targets {

template <typename TgtInfo>
class LLVM_LIBRARY_VISIBILITY OSTargetInfo : public TgtInfo {

public:
  OSTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : TgtInfo(Triple, Opts) {}

};

// CloudABI Target
template <typename Target>
class LLVM_LIBRARY_VISIBILITY CloudABITargetInfo : public OSTargetInfo<Target> {

public:
  CloudABITargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : OSTargetInfo<Target>(Triple, Opts) {}
};

// Ananas target
template <typename Target>
class LLVM_LIBRARY_VISIBILITY AnanasTargetInfo : public OSTargetInfo<Target> {

public:
  AnanasTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : OSTargetInfo<Target>(Triple, Opts) {}
};

void getDarwinDefines(const llvm::Triple &Triple, StringRef &PlatformName,
                      VersionTuple &PlatformMinVersion);

template <typename Target>
class LLVM_LIBRARY_VISIBILITY DarwinTargetInfo : public OSTargetInfo<Target> {

public:
  DarwinTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : OSTargetInfo<Target>(Triple, Opts) {
    // By default, no TLS, and we whitelist permitted architecture/OS
    // combinations.
    this->TLSSupported = false;

    if (Triple.isMacOSX())
      this->TLSSupported = !Triple.isMacOSXVersionLT(10, 7);
    else if (Triple.isiOS()) {
      // 64-bit iOS supported it from 8 onwards, 32-bit device from 9 onwards,
      // 32-bit simulator from 10 onwards.
      if (Triple.isArch64Bit())
        this->TLSSupported = !Triple.isOSVersionLT(8);
      else if (Triple.isArch32Bit()) {
        if (!Triple.isSimulatorEnvironment())
          this->TLSSupported = !Triple.isOSVersionLT(9);
        else
          this->TLSSupported = !Triple.isOSVersionLT(10);
      }
    } else if (Triple.isWatchOS()) {
      if (!Triple.isSimulatorEnvironment())
        this->TLSSupported = !Triple.isOSVersionLT(2);
      else
        this->TLSSupported = !Triple.isOSVersionLT(3);
    }

    this->MCountName = "\01mcount";
  }

  std::string isValidSectionSpecifier(StringRef SR) const override {
    // Let MCSectionMachO validate this.
    StringRef Segment, Section;
    unsigned TAA, StubSize;
    bool HasTAA;
    return llvm::MCSectionMachO::ParseSectionSpecifier(SR, Segment, Section,
                                                       TAA, HasTAA, StubSize);
  }

  const char *getStaticInitSectionSpecifier() const override {
    // FIXME: We should return 0 when building kexts.
    return "__TEXT,__StaticInit,regular,pure_instructions";
  }

  /// Darwin does not support protected visibility.  Darwin's "default"
  /// is very similar to ELF's "protected";  Darwin requires a "weak"
  /// attribute on declarations that can be dynamically replaced.
  bool hasProtectedVisibility() const override { return false; }

  unsigned getExnObjectAlignment() const override {
    // Older versions of libc++abi guarantee an alignment of only 8-bytes for
    // exception objects because of a bug in __cxa_exception that was
    // eventually fixed in r319123.
    llvm::VersionTuple MinVersion;
    const llvm::Triple &T = this->getTriple();

    // Compute the earliest OS versions that have the fix to libc++abi.
    switch (T.getOS()) {
    case llvm::Triple::Darwin:
    case llvm::Triple::MacOSX: // Earliest supporting version is 10.14.
      MinVersion = llvm::VersionTuple(10U, 14U);
      break;
    case llvm::Triple::IOS:
    case llvm::Triple::TvOS: // Earliest supporting version is 12.0.0.
      MinVersion = llvm::VersionTuple(12U);
      break;
    case llvm::Triple::WatchOS: // Earliest supporting version is 5.0.0.
      MinVersion = llvm::VersionTuple(5U);
      break;
    default:
      llvm_unreachable("Unexpected OS");
    }

    unsigned Major, Minor, Micro;
    T.getOSVersion(Major, Minor, Micro);
    if (llvm::VersionTuple(Major, Minor, Micro) < MinVersion)
      return 64;
    return OSTargetInfo<Target>::getExnObjectAlignment();
  }

  TargetInfo::IntType getLeastIntTypeByWidth(unsigned BitWidth,
                                             bool IsSigned) const final {
    // Darwin uses `long long` for `int_least64_t` and `int_fast64_t`.
    return BitWidth == 64
               ? (IsSigned ? TargetInfo::SignedLongLong
                           : TargetInfo::UnsignedLongLong)
               : TargetInfo::getLeastIntTypeByWidth(BitWidth, IsSigned);
  }
};

// DragonFlyBSD Target
template <typename Target>
class LLVM_LIBRARY_VISIBILITY DragonFlyBSDTargetInfo
    : public OSTargetInfo<Target> {

public:
  DragonFlyBSDTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : OSTargetInfo<Target>(Triple, Opts) {
    switch (Triple.getArch()) {
    default:
    case llvm::Triple::x86:
    case llvm::Triple::x86_64:
      this->MCountName = ".mcount";
      break;
    }
  }
};

#ifndef FREEBSD_CC_VERSION
#define FREEBSD_CC_VERSION 0U
#endif

// FreeBSD Target
template <typename Target>
class LLVM_LIBRARY_VISIBILITY FreeBSDTargetInfo : public OSTargetInfo<Target> {

public:
  FreeBSDTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : OSTargetInfo<Target>(Triple, Opts) {
    switch (Triple.getArch()) {
    default:
    case llvm::Triple::x86:
    case llvm::Triple::x86_64:
      this->MCountName = ".mcount";
      break;
    case llvm::Triple::mips:
    case llvm::Triple::mipsel:
    case llvm::Triple::ppc:
    case llvm::Triple::ppc64:
    case llvm::Triple::ppc64le:
      this->MCountName = "_mcount";
      break;
    case llvm::Triple::arm:
      this->MCountName = "__mcount";
      break;
    }
  }
};

// GNU/kFreeBSD Target
template <typename Target>
class LLVM_LIBRARY_VISIBILITY KFreeBSDTargetInfo : public OSTargetInfo<Target> {

public:
  KFreeBSDTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : OSTargetInfo<Target>(Triple, Opts) {}
};

// Haiku Target
template <typename Target>
class LLVM_LIBRARY_VISIBILITY HaikuTargetInfo : public OSTargetInfo<Target> {

public:
  HaikuTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : OSTargetInfo<Target>(Triple, Opts) {
    this->SizeType = TargetInfo::UnsignedLong;
    this->IntPtrType = TargetInfo::SignedLong;
    this->PtrDiffType = TargetInfo::SignedLong;
    this->ProcessIDType = TargetInfo::SignedLong;
    this->TLSSupported = false;
    switch (Triple.getArch()) {
    default:
      break;
    case llvm::Triple::x86:
    case llvm::Triple::x86_64:
      this->HasFloat128 = true;
      break;
    }
  }
};

// Hurd target
template <typename Target>
class LLVM_LIBRARY_VISIBILITY HurdTargetInfo : public OSTargetInfo<Target> {

public:
  HurdTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : OSTargetInfo<Target>(Triple, Opts) {}
};

// Minix Target
template <typename Target>
class LLVM_LIBRARY_VISIBILITY MinixTargetInfo : public OSTargetInfo<Target> {

public:
  MinixTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : OSTargetInfo<Target>(Triple, Opts) {}
};

// Linux target
template <typename Target>
class LLVM_LIBRARY_VISIBILITY LinuxTargetInfo : public OSTargetInfo<Target> {

public:
  LinuxTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : OSTargetInfo<Target>(Triple, Opts) {
    this->WIntType = TargetInfo::UnsignedInt;

    switch (Triple.getArch()) {
    default:
      break;
    case llvm::Triple::mips:
    case llvm::Triple::mipsel:
    case llvm::Triple::mips64:
    case llvm::Triple::mips64el:
    case llvm::Triple::ppc:
    case llvm::Triple::ppc64:
    case llvm::Triple::ppc64le:
      this->MCountName = "_mcount";
      break;
    case llvm::Triple::x86:
    case llvm::Triple::x86_64:
      this->HasFloat128 = true;
      break;
    }
  }

  const char *getStaticInitSectionSpecifier() const override {
    return ".text.startup";
  }
};

// NetBSD Target
template <typename Target>
class LLVM_LIBRARY_VISIBILITY NetBSDTargetInfo : public OSTargetInfo<Target> {

public:
  NetBSDTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : OSTargetInfo<Target>(Triple, Opts) {
    this->MCountName = "__mcount";
  }
};

// OpenBSD Target
template <typename Target>
class LLVM_LIBRARY_VISIBILITY OpenBSDTargetInfo : public OSTargetInfo<Target> {

public:
  OpenBSDTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : OSTargetInfo<Target>(Triple, Opts) {
    switch (Triple.getArch()) {
    case llvm::Triple::x86:
    case llvm::Triple::x86_64:
      this->HasFloat128 = true;
      LLVM_FALLTHROUGH;
    default:
      this->MCountName = "__mcount";
      break;
    case llvm::Triple::mips64:
    case llvm::Triple::mips64el:
    case llvm::Triple::ppc:
    case llvm::Triple::sparcv9:
      this->MCountName = "_mcount";
      break;
    }
  }
};

// PSP Target
template <typename Target>
class LLVM_LIBRARY_VISIBILITY PSPTargetInfo : public OSTargetInfo<Target> {

public:
  PSPTargetInfo(const llvm::Triple &Triple) : OSTargetInfo<Target>(Triple) {}
};

// PS3 PPU Target
template <typename Target>
class LLVM_LIBRARY_VISIBILITY PS3PPUTargetInfo : public OSTargetInfo<Target> {

public:
  PS3PPUTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : OSTargetInfo<Target>(Triple, Opts) {
    this->LongWidth = this->LongAlign = 32;
    this->PointerWidth = this->PointerAlign = 32;
    this->IntMaxType = TargetInfo::SignedLongLong;
    this->Int64Type = TargetInfo::SignedLongLong;
    this->SizeType = TargetInfo::UnsignedInt;
    this->resetDataLayout("E-m:e-p:32:32-i64:64-n32:64");
  }
};

template <typename Target>
class LLVM_LIBRARY_VISIBILITY PS4OSTargetInfo : public OSTargetInfo<Target> {

public:
  PS4OSTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : OSTargetInfo<Target>(Triple, Opts) {
    this->WCharType = TargetInfo::UnsignedShort;

    // On PS4, TLS variable cannot be aligned to more than 32 bytes (256 bits).
    this->MaxTLSAlign = 256;

    // On PS4, do not honor explicit bit field alignment,
    // as in "__attribute__((aligned(2))) int b : 1;".
    this->UseExplicitBitFieldAlignment = false;

    switch (Triple.getArch()) {
    default:
    case llvm::Triple::x86_64:
      this->MCountName = ".mcount";
      this->NewAlign = 256;
      break;
    }
  }
  TargetInfo::CallingConvCheckResult
  checkCallingConvention(CallingConv CC) const override {
    return (CC == CC_C) ? TargetInfo::CCCR_OK : TargetInfo::CCCR_Error;
  }
};

// RTEMS Target
template <typename Target>
class LLVM_LIBRARY_VISIBILITY RTEMSTargetInfo : public OSTargetInfo<Target> {

public:
  RTEMSTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : OSTargetInfo<Target>(Triple, Opts) {
    switch (Triple.getArch()) {
    default:
    case llvm::Triple::x86:
      // this->MCountName = ".mcount";
      break;
    case llvm::Triple::mips:
    case llvm::Triple::mipsel:
    case llvm::Triple::ppc:
    case llvm::Triple::ppc64:
    case llvm::Triple::ppc64le:
      // this->MCountName = "_mcount";
      break;
    case llvm::Triple::arm:
      // this->MCountName = "__mcount";
      break;
    }
  }
};

// Solaris target
template <typename Target>
class LLVM_LIBRARY_VISIBILITY SolarisTargetInfo : public OSTargetInfo<Target> {

public:
  SolarisTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : OSTargetInfo<Target>(Triple, Opts) {
    if (this->PointerWidth == 64) {
      this->WCharType = this->WIntType = this->SignedInt;
    } else {
      this->WCharType = this->WIntType = this->SignedLong;
    }
    switch (Triple.getArch()) {
    default:
      break;
    case llvm::Triple::x86:
    case llvm::Triple::x86_64:
      this->HasFloat128 = true;
      break;
    }
  }
};

// AIX Target
template <typename Target>
class AIXTargetInfo : public OSTargetInfo<Target> {

public:
  AIXTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : OSTargetInfo<Target>(Triple, Opts) {
    if (this->PointerWidth == 64) {
      this->WCharType = this->UnsignedInt;
    } else {
      this->WCharType = this->UnsignedShort;
    }
    this->UseZeroLengthBitfieldAlignment = true;
  }

  // AIX sets FLT_EVAL_METHOD to be 1.
  unsigned getFloatEvalMethod() const override { return 1; }
  bool hasInt128Type() const override { return false; }
};

// Windows target
template <typename Target>
class LLVM_LIBRARY_VISIBILITY WindowsTargetInfo : public OSTargetInfo<Target> {

public:
  WindowsTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : OSTargetInfo<Target>(Triple, Opts) {
    this->WCharType = TargetInfo::UnsignedShort;
    this->WIntType = TargetInfo::UnsignedShort;
  }
};

template <typename Target>
class LLVM_LIBRARY_VISIBILITY NaClTargetInfo : public OSTargetInfo<Target> {

public:
  NaClTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : OSTargetInfo<Target>(Triple, Opts) {
    this->LongAlign = 32;
    this->LongWidth = 32;
    this->PointerAlign = 32;
    this->PointerWidth = 32;
    this->IntMaxType = TargetInfo::SignedLongLong;
    this->Int64Type = TargetInfo::SignedLongLong;
    this->DoubleAlign = 64;
    this->LongDoubleWidth = 64;
    this->LongDoubleAlign = 64;
    this->LongLongWidth = 64;
    this->LongLongAlign = 64;
    this->SizeType = TargetInfo::UnsignedInt;
    this->PtrDiffType = TargetInfo::SignedInt;
    this->IntPtrType = TargetInfo::SignedInt;
    // RegParmMax is inherited from the underlying architecture.
    this->LongDoubleFormat = &llvm::APFloat::IEEEdouble();
    if (Triple.getArch() == llvm::Triple::arm) {
      // Handled in ARM's setABI().
    } else if (Triple.getArch() == llvm::Triple::x86) {
      this->resetDataLayout("e-m:e-p:32:32-p270:32:32-p271:32:32-p272:64:64-"
                            "i64:64-n8:16:32-S128");
    } else if (Triple.getArch() == llvm::Triple::x86_64) {
      this->resetDataLayout("e-m:e-p:32:32-p270:32:32-p271:32:32-p272:64:64-"
                            "i64:64-n8:16:32:64-S128");
    } else if (Triple.getArch() == llvm::Triple::mipsel) {
      // Handled on mips' setDataLayout.
    } else {
      assert(Triple.getArch() == llvm::Triple::le32);
      this->resetDataLayout("e-p:32:32-i64:64");
    }
  }
};

// Fuchsia Target
template <typename Target>
class LLVM_LIBRARY_VISIBILITY FuchsiaTargetInfo : public OSTargetInfo<Target> {

public:
  FuchsiaTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : OSTargetInfo<Target>(Triple, Opts) {
    this->MCountName = "__mcount";
    this->TheCXXABI.set(TargetCXXABI::Fuchsia);
  }
};

// WebAssembly target
template <typename Target>
class LLVM_LIBRARY_VISIBILITY WebAssemblyOSTargetInfo
    : public OSTargetInfo<Target> {
protected:
  void getOSDefines(const llvm::Triple &Triple) const {

  }

public:
  explicit WebAssemblyOSTargetInfo(const llvm::Triple &Triple,
                                   const TargetOptions &Opts)
      : OSTargetInfo<Target>(Triple, Opts) {
    this->MCountName = "__mcount";
    this->TheCXXABI.set(TargetCXXABI::WebAssembly);
    this->HasFloat128 = true;
  }
};

// WASI target
template <typename Target>
class LLVM_LIBRARY_VISIBILITY WASITargetInfo
    : public WebAssemblyOSTargetInfo<Target> {

public:
  explicit WASITargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : WebAssemblyOSTargetInfo<Target>(Triple, Opts) {}
};

// Emscripten target
template <typename Target>
class LLVM_LIBRARY_VISIBILITY EmscriptenTargetInfo
    : public WebAssemblyOSTargetInfo<Target> {

public:
  explicit EmscriptenTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : WebAssemblyOSTargetInfo<Target>(Triple, Opts) {}
};

} // namespace targets
} // namespace clang
#endif // LLVM_FLY_LIB_BASIC_TARGETS_OSTARGETS_H
