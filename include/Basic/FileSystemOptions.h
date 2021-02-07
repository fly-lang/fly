//===--- FileSystemOptions.h - File System Options --------------*- C++ -*-===//
//
// Part of the Fly Project, under the Apache License v2.0
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Defines the clang::FileSystemOptions interface.
///
//===----------------------------------------------------------------------===//

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
