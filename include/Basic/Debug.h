//===--------------------------------------------------------------------------------------------------------------===//
// include/Basic/Debug.cpp - Debug
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "llvm/Support/Debug.h"
#include <iostream>

#define DEBUG_TYPE "FLY_DEBUG"
#define FLY_DEBUG_PRINT_PREFIX "[Debug] "
#define FLY_DEBUG_PRINT_SEPCLASS "::"
#define FLY_DEBUG_PRINT_SEPMESS " - "
#define FLY_DEBUG_PRINT_POSTFIX "\n"

#define FLY_DEBUG_WITH_TYPE(TYPE, X)                                       \
  do { if (::llvm::DebugFlag) { X; } \
  } while (false)

#define FLY_DEBUG_MESSAGE(CLASS, METHOD, MESSAGE) \
    FLY_DEBUG_WITH_TYPE(DEBUG_TYPE, llvm::dbgs() << FLY_DEBUG_PRINT_PREFIX \
        << CLASS << FLY_DEBUG_PRINT_SEPCLASS << METHOD << FLY_DEBUG_PRINT_SEPMESS << MESSAGE \
        << FLY_DEBUG_PRINT_POSTFIX)

#define FLY_DEBUG(CLASS, METHOD) \
    FLY_DEBUG_WITH_TYPE(DEBUG_TYPE, llvm::dbgs() << FLY_DEBUG_PRINT_PREFIX \
        << CLASS << FLY_DEBUG_PRINT_SEPCLASS << METHOD \
        << FLY_DEBUG_PRINT_POSTFIX)
