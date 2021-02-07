//===--- OperatorKinds.h - C++ Overloaded Operators -------------*- C++ -*-===//
//
// Part of the Fly Project, under the Apache License v2.0
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Defines an enumeration for C++ overloaded operators.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_FLY_BASIC_OPERATORKINDS_H
#define LLVM_FLY_BASIC_OPERATORKINDS_H

namespace fly {

/// Enumeration specifying the different kinds of C++ overloaded
/// operators.
enum OverloadedOperatorKind : int {
  OO_None,                ///< Not an overloaded operator
#define OVERLOADED_OPERATOR(Name,Spelling,Token,Unary,Binary,MemberOnly) \
  OO_##Name,
#include "Basic/OperatorKinds.def"
  NUM_OVERLOADED_OPERATORS
};

/// Retrieve the spelling of the given overloaded operator, without
/// the preceding "operator" keyword.
const char *getOperatorSpelling(OverloadedOperatorKind Operator);

/// Get the other overloaded operator that the given operator can be rewritten
/// into, if any such operator exists.
inline OverloadedOperatorKind
getRewrittenOverloadedOperator(OverloadedOperatorKind Kind) {
  switch (Kind) {
  case OO_Less:
  case OO_LessEqual:
  case OO_Greater:
  case OO_GreaterEqual:
    return OO_Spaceship;

  case OO_ExclaimEqual:
    return OO_EqualEqual;

  default:
    return OO_None;
  }
}

} // end namespace clang

#endif
