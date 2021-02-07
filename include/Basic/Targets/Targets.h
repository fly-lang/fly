//===------- Targets.h - Declare target feature support ---------*- C++ -*-===//
//
// Part of the Fly Project, under the Apache License v2.0
//
//===----------------------------------------------------------------------===//
//
// This file declares things required for construction of a TargetInfo object
// from a target triple. Typically individual targets will need to include from
// here in order to get these functions if required.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_FLY_LIB_BASIC_TARGETS_H
#define LLVM_FLY_LIB_BASIC_TARGETS_H

#include "Basic/TargetInfo.h"
#include "llvm/ADT/StringRef.h"

namespace fly {
    namespace targets {

        LLVM_LIBRARY_VISIBILITY
        fly::TargetInfo *AllocateTarget(const llvm::Triple &Triple,
                                         const fly::TargetOptions &Opts);

    } // namespace targets
} // namespace clang
#endif // LLVM_FLY_LIB_BASIC_TARGETS_H
