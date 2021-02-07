//===--- PNaCl.cpp - Implement PNaCl target feature support ---------------===//
//
// Part of the Fly Project, under the Apache License v2.0
//
//===----------------------------------------------------------------------===//
//
// This file implements PNaCl TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "Basic/Targets/PNaCl.h"


using namespace fly;
using namespace fly::targets;

ArrayRef<const char *> PNaClTargetInfo::getGCCRegNames() const { return None; }

ArrayRef<TargetInfo::GCCRegAlias> PNaClTargetInfo::getGCCRegAliases() const {
  return None;
}
