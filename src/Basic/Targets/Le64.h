//===--- Le64.h - Declare Le64 target feature support -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares Le64 TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef FLY_LIB_BASIC_TARGETS_LE64_H
#define FLY_LIB_BASIC_TARGETS_LE64_H

#include "Basic/TargetInfo.h"
#include "Basic/TargetOptions.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Support/Compiler.h"

namespace fly {
namespace targets {

class LLVM_LIBRARY_VISIBILITY Le64TargetInfo : public TargetInfo {
  static const Builtin::Info BuiltinInfo[];

public:
  Le64TargetInfo(const llvm::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple) {
    NoAsmVariants = true;
    LongWidth = LongAlign = PointerWidth = PointerAlign = 64;
    MaxAtomicPromoteWidth = MaxAtomicInlineWidth = 64;
    resetDataLayout("e-m:e-v128:32-v16:16-v32:32-v96:32-n8:16:32:64-S128");
  }

  ArrayRef<Builtin::Info> getTargetBuiltins() const override;

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::PNaClABIBuiltinVaList;
  }

  const char *getClobbers() const override { return ""; }

  ArrayRef<const char *> getGCCRegNames() const override { return None; }

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    return None;
  }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override {
    return false;
  }

  bool hasProtectedVisibility() const override { return false; }
};

} // namespace targets
} // namespace clang
#endif // FLY_LIB_BASIC_TARGETS_LE64_H
