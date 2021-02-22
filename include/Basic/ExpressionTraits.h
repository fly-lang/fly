//===- ExpressionTraits.h - C++ Expression Traits Support Enums -*- C++ -*-===//
//
// Part of the Fly Project, under the Apache License v2.0
// See https://flylang.org/LICENSE.txt for license information.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Defines enumerations for expression traits intrinsics.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_FLY_BASIC_EXPRESSIONTRAITS_H
#define LLVM_FLY_BASIC_EXPRESSIONTRAITS_H

namespace fly {

  enum ExpressionTrait {
    ET_IsLValueExpr,
    ET_IsRValueExpr
  };
}

#endif
