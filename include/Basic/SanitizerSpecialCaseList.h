//===--------------------------------------------------------------------------------------------------------------===//
// include/Basic/SanitizerSpecialCaseList.h - SCL for sanitizers
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
#ifndef LLVM_FLY_BASIC_SANITIZERSPECIALCASELIST_H
#define LLVM_FLY_BASIC_SANITIZERSPECIALCASELIST_H

#include "Basic/LLVM.h"
#include "Basic/Sanitizers.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/SpecialCaseList.h"
#include "llvm/Support/VirtualFileSystem.h"
#include <memory>

namespace fly {

class SanitizerSpecialCaseList : public llvm::SpecialCaseList {
public:
  static std::unique_ptr<SanitizerSpecialCaseList>
  create(const std::vector<std::string> &Paths, llvm::vfs::FileSystem &VFS,
         std::string &Error);

  static std::unique_ptr<SanitizerSpecialCaseList>
  createOrDie(const std::vector<std::string> &Paths,
              llvm::vfs::FileSystem &VFS);

  // Query blacklisted entries if any bit in Mask matches the entry's section.
  bool inSection(SanitizerMask Mask, StringRef Prefix, StringRef Query,
                 StringRef Category = StringRef()) const;

protected:
  // Initialize SanitizerSections.
  void createSanitizerSections();

  struct SanitizerSection {
    SanitizerSection(SanitizerMask SM, SectionEntries &E)
        : Mask(SM), Entries(E){};

    SanitizerMask Mask;
    SectionEntries &Entries;
  };

  std::vector<SanitizerSection> SanitizerSections;
};

} // end namespace fly

#endif
