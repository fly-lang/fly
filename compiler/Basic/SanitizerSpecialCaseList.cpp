//===--- SanitizerSpecialCaseList.cpp - SCL for sanitizers --------------------------------------------------------===//
//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Basic/SanitizerSpecialCaseList.cpp - sanitizer special case lists
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
//
// An extension of SpecialCaseList to allowing querying sections by
// SanitizerMask.
//
//===--------------------------------------------------------------------------------------------------------------===//
#include "Basic/SanitizerSpecialCaseList.h"
#include "llvm/ADT/Twine.h"

using namespace fly;

std::unique_ptr<SanitizerSpecialCaseList>
SanitizerSpecialCaseList::create(const std::vector<std::string> &Paths,
                                 llvm::vfs::FileSystem &VFS,
                                 std::string &Error) {
  std::unique_ptr<fly::SanitizerSpecialCaseList> SSCL(
      new SanitizerSpecialCaseList());
  if (SSCL->createInternal(Paths, VFS, Error)) {
    SSCL->createSanitizerSections();
    return SSCL;
  }
  return nullptr;
}

std::unique_ptr<SanitizerSpecialCaseList>
SanitizerSpecialCaseList::createOrDie(const std::vector<std::string> &Paths,
                                      llvm::vfs::FileSystem &VFS) {
  std::string Error;
  if (auto SSCL = create(Paths, VFS, Error))
    return SSCL;
  llvm::report_fatal_error(llvm::Twine(Error));
}

void SanitizerSpecialCaseList::createSanitizerSections() {
  for (auto &S : Sections) {
    SanitizerMask Mask;
    auto &Sec = S.getValue();

#define SANITIZER(NAME, ID)                                                    \
  if (Sec.SectionMatcher->match(NAME))                                         \
    Mask |= SanitizerKind::ID;
#define SANITIZER_GROUP(NAME, ID, ALIAS) SANITIZER(NAME, ID)

#include "Basic/Sanitizers.def"
#undef SANITIZER
#undef SANITIZER_GROUP

    SanitizerSections.emplace_back(Mask, Sec.Entries);
  }
}

bool SanitizerSpecialCaseList::inSection(SanitizerMask Mask, StringRef Prefix,
                                         StringRef Query,
                                         StringRef Category) const {
  for (auto &S : SanitizerSections)
    if ((S.Mask & Mask) &&
        SpecialCaseList::inSectionBlame(S.Entries, Prefix, Query, Category))
      return true;

  return false;
}
