//===--------------------------------------------------------------------------------------------------------------===//
// include/Basic/FileSystemOptions.h - File System Options
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
///
/// \file
/// Defines the clang::FileSystemOptions interface.
///
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef LLVM_FLY_BASIC_FILESYSTEMOPTIONS_H
#define LLVM_FLY_BASIC_FILESYSTEMOPTIONS_H

#include <string>

namespace fly {

/// Keeps track of options that affect how file operations are performed.
class FileSystemOptions {
public:
  /// If set, paths are resolved as if the working directory was
  /// set to the value of WorkingDir.
  std::string WorkingDir;
};

} // end namespace clang

#endif
