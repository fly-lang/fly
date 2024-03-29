//===--- PNaCl.h - Declare PNaCl target feature support ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares PNaCl TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef FLY_LIB_BASIC_TARGETS_PNACL_H
#define FLY_LIB_BASIC_TARGETS_PNACL_H

#include "Mips.h"
#include "Basic/TargetInfo.h"
#include "Basic/TargetOptions.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Support/Compiler.h"

namespace fly {
namespace targets {

class LLVM_LIBRARY_VISIBILITY PNaClTargetInfo : public TargetInfo {
public:
  PNaClTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : TargetInfo(Triple) {
    this->LongAlign = 32;
    this->LongWidth = 32;
    this->PointerAlign = 32;
    this->PointerWidth = 32;
    this->IntMaxType = TargetInfo::SignedLongLong;
    this->Int64Type = TargetInfo::SignedLongLong;
    this->DoubleAlign = 64;
    this->LongDoubleWidth = 64;
    this->LongDoubleAlign = 64;
    this->SizeType = TargetInfo::UnsignedInt;
    this->PtrDiffType = TargetInfo::SignedInt;
    this->IntPtrType = TargetInfo::SignedInt;
    this->RegParmMax = 0; // Disallow regparm
  }

  bool hasFeature(StringRef Feature) const override {
    return Feature == "pnacl";
  }

  ArrayRef<Builtin::Info> getTargetBuiltins() const override { return None; }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::PNaClABIBuiltinVaList;
  }

  ArrayRef<const char *> getGCCRegNames() const override;

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override;

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override {
    return false;
  }

  const char *getClobbers() const override { return ""; }

  bool hasExtIntType() const override { return true; }
};

// We attempt to use PNaCl (le32) frontend and Mips32EL backend.
class LLVM_LIBRARY_VISIBILITY NaClMips32TargetInfo : public MipsTargetInfo {
public:
  NaClMips32TargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : MipsTargetInfo(Triple, Opts) {}

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::PNaClABIBuiltinVaList;
  }
};
} // namespace targets
} // namespace clang

#endif // FLY_LIB_BASIC_TARGETS_PNACL_H
