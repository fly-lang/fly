//===----- Attr.h - Enum values for C Attribute Kinds ----------*- C++ -*-===//
//
// Part of the Fly Project, under the Apache License v2.0
// See https://flylang.org/LICENSE.txt for license information.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Defines the clang::attr::Kind enum.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_FLY_BASIC_ATTRKINDS_H
#define LLVM_FLY_BASIC_ATTRKINDS_H

namespace fly {

namespace attr {

// A list of all the recognized kinds of attributes.
enum Kind {
#define ATTR(X) X,
#define ATTR_RANGE(CLASS, FIRST_NAME, LAST_NAME) \
  First##CLASS = FIRST_NAME,                    \
  Last##CLASS = LAST_NAME,
#include "Basic/AttrList.inc"
};

} // end namespace attr
} // end namespace clang

#endif
