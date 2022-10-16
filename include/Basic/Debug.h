//===--------------------------------------------------------------------------------------------------------------===//
// include/Basic/Debug.cpp - Debug
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_BASIC_DEBUG_H
#define FLY_BASIC_DEBUG_H

#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "FLY_DEBUG"
#define FLY_DEBUG_PRINT_PREFIX "[Debug] "
#define FLY_DEBUG_PRINT_SEP1 "::"
#define FLY_DEBUG_PRINT_SEP2 " - "
#define FLY_DEBUG_PRINT_POSTFIX "\n"

#define FLY_DEBUG_MESSAGE(CLASS, METHOD, MESSAGE) \
    DEBUG_WITH_TYPE(DEBUG_TYPE, llvm::dbgs() << FLY_DEBUG_PRINT_PREFIX \
        << CLASS << FLY_DEBUG_PRINT_SEP1 << METHOD << FLY_DEBUG_PRINT_SEP2 << MESSAGE \
        << FLY_DEBUG_PRINT_POSTFIX)

#define FLY_DEBUG(CLASS, METHOD) \
    DEBUG_WITH_TYPE(DEBUG_TYPE, llvm::dbgs() << FLY_DEBUG_PRINT_PREFIX \
        << CLASS << FLY_DEBUG_PRINT_SEP1 << METHOD \
        << FLY_DEBUG_PRINT_POSTFIX)

#endif