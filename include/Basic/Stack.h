//===--- Stack.h - Utilities for dealing with stack space -------*- C++ -*-===//
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Defines utilities for dealing with stack allocation and stack space.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_FLY_BASIC_STACK_H
#define LLVM_FLY_BASIC_STACK_H

#include <cstddef>

#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/Compiler.h"

namespace fly {
  /// The amount of stack space that Clang would like to be provided with.
  /// If less than this much is available, we may be unable to reach our
  /// template instantiation depth limit and other similar limits.
  constexpr size_t DesiredStackSize = 8 << 20;

  /// Call this once on each thread, as soon after starting the thread as
  /// feasible, to note the approximate address of the bottom of the stack.
  void noteBottomOfStack();

  /// Determine whether the stack is nearly exhausted.
  bool isStackNearlyExhausted();

  void runWithSufficientStackSpaceSlow(llvm::function_ref<void()> Diag,
                                       llvm::function_ref<void()> Fn);

  /// Run a given function on a stack with "sufficient" space. If stack space
  /// is insufficient, calls Diag to emit a diagnostic before calling Fn.
  inline void runWithSufficientStackSpace(llvm::function_ref<void()> Diag,
                                          llvm::function_ref<void()> Fn) {
#ifdef LLVM_ENABLE_THREADS
    if (LLVM_UNLIKELY(isStackNearlyExhausted()))
      runWithSufficientStackSpaceSlow(Diag, Fn);
    else
      Fn();
#else
    if (LLVM_UNLIKELY(isStackNearlyExhausted()))
      Diag();
    Fn();
#endif
  }
} // end namespace clang

#endif // LLVM_FLY_BASIC_STACK_H
