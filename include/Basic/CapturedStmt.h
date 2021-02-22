//===--- CapturedStmt.h - Types for CapturedStmts ---------------*- C++ -*-===//
//
// Part of the Fly Project, under the Apache License v2.0
// See https://flylang.org/LICENSE.txt for license information.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//


#ifndef LLVM_FLY_BASIC_CAPTUREDSTMT_H
#define LLVM_FLY_BASIC_CAPTUREDSTMT_H

namespace fly {

/// The different kinds of captured statement.
enum CapturedRegionKind {
  CR_Default,
  CR_ObjCAtFinally,
  CR_OpenMP
};

} // end namespace clang

#endif // LLVM_FLY_BASIC_CAPTUREDSTMT_H
