//===--- BPF.cpp - Implement BPF target feature support -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements BPF TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "BPF.h"
#include "Basic/Targets.h"
#include "Basic/TargetBuiltins.h"
#include "llvm/ADT/StringRef.h"

using namespace fly;
using namespace fly::targets;

const Builtin::Info BPFTargetInfo::BuiltinInfo[] = {
#define BUILTIN(ID, TYPE, ATTRS)                                               \
  {#ID, TYPE, ATTRS, nullptr, ALL_LANGUAGES, nullptr},
#include "Basic/BuiltinsBPF.def"
};

static constexpr llvm::StringLiteral ValidCPUNames[] = {"generic", "v1", "v2",
                                                        "v3", "probe"};

bool BPFTargetInfo::isValidCPUName(StringRef Name) const {
  return llvm::find(ValidCPUNames, Name) != std::end(ValidCPUNames);
}

void BPFTargetInfo::fillValidCPUList(SmallVectorImpl<StringRef> &Values) const {
  Values.append(std::begin(ValidCPUNames), std::end(ValidCPUNames));
}

ArrayRef<Builtin::Info> BPFTargetInfo::getTargetBuiltins() const {
  return llvm::makeArrayRef(BuiltinInfo, fly::BPF::LastTSBuiltin -
                                             Builtin::FirstTSBuiltin);
}
