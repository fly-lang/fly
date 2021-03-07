//===--------------------------------------------------------------------------------------------------------------===//
// include/Driver/Options.h - Option info & table
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_DRIVER_OPTIONS_H
#define FLY_DRIVER_OPTIONS_H

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
            enum FlyFlags {
                Unsupported = (1 << 4),
                CoreOption = (1 << 5),
                Ignored = (1 << 6)
            };

            enum ID {
                OPT_INVALID = 0, // This is not an option ID.
                #define OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, ALIASARGS, FLAGS, PARAM, HELPTEXT, METAVAR, VALUES)                                      \
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
