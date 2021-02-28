//===- Sanitizers.cpp - C Language Family Language Options ----------------===//
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//
//
//  This file defines the classes from Sanitizers.h
//
//===----------------------------------------------------------------------===//

#include "Basic/Sanitizers.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/StringSwitch.h"

using namespace fly;

// Once LLVM switches to C++17, the constexpr variables can be inline and we
// won't need this.
#define SANITIZER(NAME, ID) constexpr SanitizerMask SanitizerKind::ID;
#define SANITIZER_GROUP(NAME, ID, ALIAS)                                       \
  constexpr SanitizerMask SanitizerKind::ID;                                   \
  constexpr SanitizerMask SanitizerKind::ID##Group;
#include "Basic/Sanitizers.def"

SanitizerMask fly::parseSanitizerValue(StringRef Value, bool AllowGroups) {
  SanitizerMask ParsedKind = llvm::StringSwitch<SanitizerMask>(Value)
#define SANITIZER(NAME, ID) .Case(NAME, SanitizerKind::ID)
#define SANITIZER_GROUP(NAME, ID, ALIAS)                                       \
  .Case(NAME, AllowGroups ? SanitizerKind::ID##Group : SanitizerMask())
#include "Basic/Sanitizers.def"
    .Default(SanitizerMask());
  return ParsedKind;
}

SanitizerMask fly::expandSanitizerGroups(SanitizerMask Kinds) {
#define SANITIZER(NAME, ID)
#define SANITIZER_GROUP(NAME, ID, ALIAS)                                       \
  if (Kinds & SanitizerKind::ID##Group)                                        \
    Kinds |= SanitizerKind::ID;
#include "Basic/Sanitizers.def"
  return Kinds;
}

llvm::hash_code SanitizerMask::hash_value() const {
  return llvm::hash_combine_range(&maskLoToHigh[0], &maskLoToHigh[kNumElem]);
}

namespace fly {
llvm::hash_code hash_value(const fly::SanitizerMask &Arg) {
  return Arg.hash_value();
}
} // namespace clang
