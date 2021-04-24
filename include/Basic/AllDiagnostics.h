//===--------------------------------------------------------------------------------------------------------------===//
// include/Basic/AllDiagnostics.h - Aggregate Diagnostic headers
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
///
/// \file
/// Includes all the separate Diagnostic headers & some related helpers.
///
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef LLVM_FLY_BASIC_ALLDIAGNOSTICS_H
#define LLVM_FLY_BASIC_ALLDIAGNOSTICS_H

#include "Basic/Diagnostic.h"

namespace fly {
template <size_t SizeOfStr, typename FieldType>
class StringSizerHelper {
  static_assert(SizeOfStr <= FieldType(~0U), "Field too small!");
public:
  enum { Size = SizeOfStr };
};
} // end namespace fly

#define STR_SIZE(str, fieldTy) fly::StringSizerHelper<sizeof(str)-1, \
                                                        fieldTy>::Size

#endif
