//===--------------------------------------------------------------------------------------------------------------===//
// include/Basic/SanitizerBlacklist.h - Blacklist for sanitizers
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
//
// User-provided blacklist used to disable/alter instrumentation done in
// sanitizers.
//
//===--------------------------------------------------------------------------------------------------------------===//
#ifndef LLVM_FLY_BASIC_SANITIZERBLACKLIST_H
#define LLVM_FLY_BASIC_SANITIZERBLACKLIST_H

#include "Basic/LLVM.h"
#include "Basic/SanitizerSpecialCaseList.h"
#include "Basic/Sanitizers.h"
#include "Basic/SourceLocation.h"
#include "Basic/SourceManager.h"
#include "llvm/ADT/StringRef.h"
#include <memory>

namespace fly {

class SanitizerBlacklist {
  std::unique_ptr<SanitizerSpecialCaseList> SSCL;
  SourceManager &SM;

public:
  SanitizerBlacklist(const std::vector<std::string> &BlacklistPaths,
                     SourceManager &SM);
  bool isBlacklistedGlobal(SanitizerMask Mask, StringRef GlobalName,
                           StringRef Category = StringRef()) const;
  bool isBlacklistedType(SanitizerMask Mask, StringRef MangledTypeName,
                         StringRef Category = StringRef()) const;
  bool isBlacklistedFunction(SanitizerMask Mask, StringRef FunctionName) const;
  bool isBlacklistedFile(SanitizerMask Mask, StringRef FileName,
                         StringRef Category = StringRef()) const;
  bool isBlacklistedLocation(SanitizerMask Mask, SourceLocation Loc,
                             StringRef Category = StringRef()) const;
};

}  // end namespace clang

#endif
