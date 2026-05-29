//===--------------------------------------------------------------------------------------------------------------===//
// include/Parser/LiteralSupport.h - literal token parsing
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef LLVM_FLY_LEX_LITERALSUPPORT_H
#define LLVM_FLY_LEX_LITERALSUPPORT_H

#include "Basic/LLVM.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"

namespace fly {

/// Copy characters from Input to Buf, expanding any UCNs (\uXXXX / \UXXXXXXXX).
void expandUCNs(SmallVectorImpl<char> &Buf, StringRef Input);

} // end namespace fly

#endif
