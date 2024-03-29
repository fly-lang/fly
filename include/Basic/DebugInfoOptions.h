//===--------------------------------------------------------------------------------------------------------------===//
// include/Basic/DebugInfoOptions.h - Debug Info Emission Types
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef LLVM_FLY_BASIC_DEBUGINFOOPTIONS_H
#define LLVM_FLY_BASIC_DEBUGINFOOPTIONS_H

namespace fly {
namespace codegenoptions {

enum DebugInfoFormat {
  DIF_DWARF,
  DIF_CodeView,
};

enum DebugInfoKind {
  /// Don't generate debug info.
  NoDebugInfo,

  /// Emit location information but do not generate debug info in the output.
  /// This is useful in cases where the backend wants to track source
  /// locations for instructions without actually emitting debug info for them
  /// (e.g., when -Rpass is used).
  LocTrackingOnly,

  /// Emit only debug directives with the line numbers data
  DebugDirectivesOnly,

  /// Emit only debug info necessary for generating line number tables
  /// (-gline-tables-only).
  DebugLineTablesOnly,

  /// Limit generated debug info for classes to reduce size. This emits class
  /// type info only where the constructor is emitted, if it is a class that
  /// has a constructor.
  DebugInfoConstructor,

  /// Limit generated debug info to reduce size (-fno-standalone-debug). This
  /// emits forward decls for types that could be replaced with forward decls in
  /// the source code. For dynamic C++ classes type info is only emitted into
  /// the module that contains the classe's vtable.
  LimitedDebugInfo,

  /// GenStmt complete debug info.
  FullDebugInfo
};

} // end namespace codegenoptions
} // end namespace fly

#endif
