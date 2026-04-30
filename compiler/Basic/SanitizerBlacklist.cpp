//===--- SanitizerBlacklist.cpp - Blacklist for sanitizers ----------------===//
//===--------------------------------------------------------------------------------------------------------------===//
// include/Basic/AddressSpaces.h - Language-specific address spaces
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
#include "Basic/SanitizerBlacklist.h"

using namespace fly;

SanitizerBlacklist::SanitizerBlacklist(
    const std::vector<std::string> &BlacklistPaths, SourceManager &SM)
    : SSCL(SanitizerSpecialCaseList::createOrDie(
          BlacklistPaths, SM.getFileManager().getVirtualFileSystem())),
      SM(SM) {}

bool SanitizerBlacklist::isBlacklistedGlobal(SanitizerMask Mask,
                                             StringRef GlobalName,
                                             StringRef Category) const {
  return SSCL->inSection(Mask, "global", GlobalName, Category);
}

bool SanitizerBlacklist::isBlacklistedType(SanitizerMask Mask,
                                           StringRef MangledTypeName,
                                           StringRef Category) const {
  return SSCL->inSection(Mask, "type", MangledTypeName, Category);
}

bool SanitizerBlacklist::isBlacklistedFunction(SanitizerMask Mask,
                                               StringRef FunctionName) const {
  return SSCL->inSection(Mask, "fun", FunctionName);
}

bool SanitizerBlacklist::isBlacklistedFile(SanitizerMask Mask,
                                           StringRef FileName,
                                           StringRef Category) const {
  return SSCL->inSection(Mask, "src", FileName, Category);
}

bool SanitizerBlacklist::isBlacklistedLocation(SanitizerMask Mask,
                                               SourceLocation Loc,
                                               StringRef Category) const {
  return Loc.isValid() &&
         isBlacklistedFile(Mask, SM.getFilename(SM.getFileLoc(Loc)), Category);
}

