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
                OPT_INPUT,
                OPT_UNKNOWN,
                OPT_EMIT_AS,
                OPT_EMIT_BC,
                OPT_EMIT_LL,
                OPT_DEBUG,
                OPT_FTIME_REPORT,
                OPT_HEADER_GENERATOR,
                OPT_HELP,
                OPT_NO_WARNING,
                OPT_LOG_FILE,
                OPT_MC_MODEL,
                OPT_MTHREAD_MODEL,
                OPT_NO_OUTPUT,
                OPT_OUTPUT,
                OPT_OUTPUT_LIB,
                OPT_PRINT_STATS,
                OPT_STATS_FILE,
                OPT_TARGET_CPU,
                OPT_TARGET,
                OPT_VERSION_SHORT,
                OPT_VERSION,
                OPT_VERBOSE,
                OPT_WORKING_DIR,
            };
        }

        const llvm::opt::OptTable &getDriverOptTable();
    }
}

#endif
