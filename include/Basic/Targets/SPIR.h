//===--- SPIR.h - Declare SPIR target feature support -----------*- C++ -*-===//
//
// Part of the Fly Project, under the Apache License v2.0
// See https://flylang.org/LICENSE.txt for license information.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//
//
// This file declares SPIR TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_FLY_LIB_BASIC_TARGETS_SPIR_H
#define LLVM_FLY_LIB_BASIC_TARGETS_SPIR_H

#include "Basic/TargetInfo.h"
#include "Basic/TargetOptions.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Support/Compiler.h"

namespace fly {
namespace targets {

static const unsigned SPIRAddrSpaceMap[] = {
    0, // Default
    1, // opencl_global
    3, // opencl_local
    2, // opencl_constant
    0, // opencl_private
    4, // opencl_generic
    0, // cuda_device
    0, // cuda_constant
    0, // cuda_shared
    0, // ptr32_sptr
    0, // ptr32_uptr
    0  // ptr64
};

class LLVM_LIBRARY_VISIBILITY SPIRTargetInfo : public TargetInfo {
public:
  SPIRTargetInfo(const llvm::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple) {
    assert(getTriple().getOS() == llvm::Triple::UnknownOS &&
           "SPIR target must use unknown OS");
    assert(getTriple().getEnvironment() == llvm::Triple::UnknownEnvironment &&
           "SPIR target must use unknown environment type");
    TLSSupported = false;
    VLASupported = false;
    LongWidth = LongAlign = 64;
    AddrSpaceMap = &SPIRAddrSpaceMap;
    UseAddrSpaceMapMangling = true;
    HasLegalHalfType = true;
    HasFloat16 = true;
    // Define available target features
    // These must be defined in sorted order!
    NoAsmVariants = true;
  }



  bool hasFeature(StringRef Feature) const override {
    return Feature == "spir";
  }

  // SPIR supports the half type and the only llvm intrinsic allowed in SPIR is
  // memcpy as per section 3 of the SPIR spec.
  bool useFP16ConversionIntrinsics() const override { return false; }

  ArrayRef<Builtin::Info> getTargetBuiltins() const override { return None; }

  const char *getClobbers() const override { return ""; }

  ArrayRef<const char *> getGCCRegNames() const override { return None; }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &info) const override {
    return true;
  }

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    return None;
  }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::VoidPtrBuiltinVaList;
  }

  CallingConvCheckResult checkCallingConvention(CallingConv CC) const override {
    return (CC == CC_SpirFunction || CC == CC_OpenCLKernel) ? CCCR_OK
                                                            : CCCR_Warning;
  }

  CallingConv getDefaultCallingConv() const override {
    return CC_SpirFunction;
  }

};
class LLVM_LIBRARY_VISIBILITY SPIR32TargetInfo : public SPIRTargetInfo {
public:
  SPIR32TargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : SPIRTargetInfo(Triple, Opts) {
    PointerWidth = PointerAlign = 32;
    SizeType = TargetInfo::UnsignedInt;
    PtrDiffType = IntPtrType = TargetInfo::SignedInt;
    resetDataLayout("e-p:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-"
                    "v96:128-v192:256-v256:256-v512:512-v1024:1024");
  }


};

class LLVM_LIBRARY_VISIBILITY SPIR64TargetInfo : public SPIRTargetInfo {
public:
  SPIR64TargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : SPIRTargetInfo(Triple, Opts) {
    PointerWidth = PointerAlign = 64;
    SizeType = TargetInfo::UnsignedLong;
    PtrDiffType = IntPtrType = TargetInfo::SignedLong;
    resetDataLayout("e-i64:64-v16:16-v24:32-v32:32-v48:64-"
                    "v96:128-v192:256-v256:256-v512:512-v1024:1024");
  }


};
} // namespace targets
} // namespace clang
#endif // LLVM_FLY_LIB_BASIC_TARGETS_SPIR_H
