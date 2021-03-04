//===--- BPF.h - Declare BPF target feature support -----------------------------------------------------*- C++ -*-===//
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
//
// This file declares BPF TargetInfo objects.
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef LLVM_FLY_LIB_BASIC_TARGETS_BPF_H
#define LLVM_FLY_LIB_BASIC_TARGETS_BPF_H

#include "Basic/TargetInfo.h"
#include "Basic/TargetOptions.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Support/Compiler.h"

namespace fly {
namespace targets {

class LLVM_LIBRARY_VISIBILITY BPFTargetInfo : public TargetInfo {
  static const Builtin::Info BuiltinInfo[];

public:
  BPFTargetInfo(const llvm::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple) {
    LongWidth = LongAlign = PointerWidth = PointerAlign = 64;
    SizeType = UnsignedLong;
    PtrDiffType = SignedLong;
    IntPtrType = SignedLong;
    IntMaxType = SignedLong;
    Int64Type = SignedLong;
    RegParmMax = 5;
    if (Triple.getArch() == llvm::Triple::bpfeb) {
      resetDataLayout("E-m:e-p:64:64-i64:64-n32:64-S128");
    } else {
      resetDataLayout("e-m:e-p:64:64-i64:64-n32:64-S128");
    }
    MaxAtomicPromoteWidth = 64;
    MaxAtomicInlineWidth = 64;
    TLSSupported = false;
  }



  bool hasFeature(StringRef Feature) const override {
    return Feature == "bpf" || Feature == "alu32" || Feature == "dwarfris";
  }

  void setFeatureEnabled(llvm::StringMap<bool> &Features, StringRef Name,
                         bool Enabled) const override {
    Features[Name] = Enabled;
  }

  ArrayRef<Builtin::Info> getTargetBuiltins() const override;

  const char *getClobbers() const override { return ""; }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::VoidPtrBuiltinVaList;
  }

  bool isValidGCCRegisterName(StringRef Name) const override { return true; }
  ArrayRef<const char *> getGCCRegNames() const override { return None; }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &info) const override {
    return true;
  }

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    return None;
  }

  bool allowDebugInfoForExternalVar() const override { return true; }

  CallingConvCheckResult checkCallingConvention(CallingConv CC) const override {
    switch (CC) {
    default:
      return CCCR_Warning;
    case CC_C:
    case CC_OpenCLKernel:
      return CCCR_OK;
    }
  }

  bool isValidCPUName(StringRef Name) const override;

  void fillValidCPUList(SmallVectorImpl<StringRef> &Values) const override;

  bool setCPU(const std::string &Name) override {
    StringRef CPUName(Name);
    return isValidCPUName(CPUName);
  }
};
} // namespace targets
} // namespace clang
#endif // LLVM_FLY_LIB_BASIC_TARGETS_BPF_H
