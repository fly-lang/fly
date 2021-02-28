//===- DiagnosticCategories.h - Diagnostic Categories Enumerators-*- C++ -*===//
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_FLY_BASIC_DIAGNOSTICCATEGORIES_H
#define LLVM_FLY_BASIC_DIAGNOSTICCATEGORIES_H

namespace fly {
  namespace diag {
    enum {
#define GET_CATEGORY_TABLE
#define CATEGORY(X, ENUM) ENUM,
#include "Basic/DiagnosticGroups.inc"
#undef CATEGORY
#undef GET_CATEGORY_TABLE
      DiagCat_NUM_CATEGORIES
    };
  }  // end namespace diag
}  // end namespace clang

#endif
