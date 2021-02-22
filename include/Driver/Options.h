//===----------------------------------------------------------------------===//
// Driver/Options.h - Option info & table
//
// Part of the Fly Project, under the Apache License v2.0
// See https://flylang.org/LICENSE.txt for license information.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_DRIVER_OPTIONS_H
#define LLVM_CLANG_DRIVER_OPTIONS_H

#include <memory>

namespace llvm {
namespace opt {
class OptTable;
}
}

namespace fly {
namespace driver {

namespace options {
/// Flags specifically for fly options.  Must not overlap with
/// llvm::opt::DriverFlag.
enum ClangFlags {
  DriverOption = (1 << 4),
  LinkerInput = (1 << 5),
  NoArgumentUnused = (1 << 6),
  Unsupported = (1 << 7),
  CoreOption = (1 << 8),
  CLOption = (1 << 9),
  CC1Option = (1 << 10),
  CC1AsOption = (1 << 11),
  NoDriverOption = (1 << 12),
  Ignored = (1 << 13)
};

  enum ID {
      OPT_INVALID = 0, // This is not an option ID.
  #define OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, ALIASARGS, FLAGS, PARAM,  \
                 HELPTEXT, METAVAR, VALUES)                                      \
    OPT_##ID,
  #include "Options.inc"
      LastOption
  #undef OPTION
    };
  }

  const llvm::opt::OptTable &getDriverOptTable();
}
}

#endif
