//===--- Le64.cpp - Implement Le64 target feature support -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements Le64 TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "Le64.h"
#include "Basic/Targets.h"
#include "Basic/Builtins.h"

#include "Basic/TargetBuiltins.h"

using namespace fly;
using namespace fly::targets;

const Builtin::Info Le64TargetInfo::BuiltinInfo[] = {
#define BUILTIN(ID, TYPE, ATTRS)                                               \
  {#ID, TYPE, ATTRS, nullptr, ALL_LANGUAGES, nullptr},
#include "Basic/BuiltinsLe64.def"
};

ArrayRef<Builtin::Info> Le64TargetInfo::getTargetBuiltins() const {
  return llvm::makeArrayRef(BuiltinInfo, fly::Le64::LastTSBuiltin -
                                             Builtin::FirstTSBuiltin);
}
