//===--- VE.cpp - Implement VE target feature support ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements VE TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "VE.h"
#include "Basic/Builtins.h"

#include "Basic/TargetBuiltins.h"

using namespace fly;
using namespace fly::targets;

ArrayRef<Builtin::Info> VETargetInfo::getTargetBuiltins() const {
  return ArrayRef<Builtin::Info>();
}
