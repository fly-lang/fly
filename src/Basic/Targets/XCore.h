//===--- XCore.h - Declare XCore target feature support ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares XCore TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef FLY_LIB_BASIC_TARGETS_XCORE_H
#define FLY_LIB_BASIC_TARGETS_XCORE_H

#include "Basic/TargetInfo.h"
#include "Basic/TargetOptions.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Support/Compiler.h"

namespace fly {
namespace targets {

class LLVM_LIBRARY_VISIBILITY XCoreTargetInfo : public TargetInfo {
  static const Builtin::Info BuiltinInfo[];

public:
  XCoreTargetInfo(const llvm::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple) {
    NoAsmVariants = true;
    LongLongAlign = 32;
    SuitableAlign = 32;
    DoubleAlign = LongDoubleAlign = 32;
    SizeType = UnsignedInt;
    PtrDiffType = SignedInt;
    IntPtrType = SignedInt;
    WCharType = UnsignedChar;
    WIntType = UnsignedInt;
    UseZeroLengthBitfieldAlignment = true;
    resetDataLayout("e-m:e-p:32:32-i1:8:32-i8:8:32-i16:16:32-i64:32"
                    "-f64:32-a:0:32-n32");
  }

  ArrayRef<Builtin::Info> getTargetBuiltins() const override;

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::VoidPtrBuiltinVaList;
  }

  const char *getClobbers() const override { return ""; }

  ArrayRef<const char *> getGCCRegNames() const override {
    static const char *const GCCRegNames[] = {
        "r0", "r1", "r2",  "r3",  "r4", "r5", "r6", "r7",
        "r8", "r9", "r10", "r11", "cp", "dp", "sp", "lr"
    };
    return llvm::makeArrayRef(GCCRegNames);
  }

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    return None;
  }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override {
    return false;
  }

  int getEHDataRegisterNumber(unsigned RegNo) const override {
    // R0=ExceptionPointerRegister R1=ExceptionSelectorRegister
    return (RegNo < 2) ? RegNo : -1;
  }

  bool allowsLargerPreferedTypeAlignment() const override { return false; }

  bool hasExtIntType() const override { return true; }
};
} // namespace targets
} // namespace clang
#endif // FLY_LIB_BASIC_TARGETS_XCORE_H
