//===--- BitmaskEnum.h - wrapper of LLVM's bitmask enum facility-*- C++ -*-===//
//
// Part of the Fly Project, under the Apache License v2.0
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Provides LLVM's BitmaskEnum facility to enumeration types declared in
/// namespace clang.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_FLY_BASIC_BITMASKENUM_H
#define LLVM_FLY_BASIC_BITMASKENUM_H

#include "llvm/ADT/BitmaskEnum.h"

namespace fly {
  LLVM_ENABLE_BITMASK_ENUMS_IN_NAMESPACE();
}

#endif
