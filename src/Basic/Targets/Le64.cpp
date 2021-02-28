//===--- Le64.cpp - Implement Le64 target feature support -----------------===//
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//
//
// This file implements Le64 TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "Basic/Targets/Le64.h"
#include "Basic/Targets/Targets.h"
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
