//===--- MSP430.cpp - Implement MSP430 target feature support -------------===//
//
// Part of the Fly Project, under the Apache License v2.0
//
//===----------------------------------------------------------------------===//
//
// This file implements MSP430 TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "Basic/Targets/MSP430.h"


using namespace fly;
using namespace fly::targets;

const char *const MSP430TargetInfo::GCCRegNames[] = {
    "r0", "r1", "r2",  "r3",  "r4",  "r5",  "r6",  "r7",
    "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
};

ArrayRef<const char *> MSP430TargetInfo::getGCCRegNames() const {
  return llvm::makeArrayRef(GCCRegNames);
}
