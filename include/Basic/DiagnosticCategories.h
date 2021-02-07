//===- DiagnosticCategories.h - Diagnostic Categories Enumerators-*- C++ -*===//
//
// Part of the Fly Project, under the Apache License v2.0
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
