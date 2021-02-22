//===- CommentOptions.h - Options for parsing comments ----------*- C++ -*-===//
//
// Part of the Fly Project, under the Apache License v2.0
// See https://flylang.org/LICENSE.txt for license information.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Defines the clang::CommentOptions interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_FLY_BASIC_COMMENTOPTIONS_H
#define LLVM_FLY_BASIC_COMMENTOPTIONS_H

#include <string>
#include <vector>

namespace fly {

/// Options for controlling comment parsing.
struct CommentOptions {
  using BlockCommandNamesTy = std::vector<std::string>;

  /// Command names to treat as block commands in comments.
  /// Should not include the leading backslash.
  BlockCommandNamesTy BlockCommandNames;

  /// Treat ordinary comments as documentation comments.
  bool ParseAllComments = false;

  CommentOptions() = default;
};

} // namespace clang

#endif // LLVM_FLY_BASIC_COMMENTOPTIONS_H
