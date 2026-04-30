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

            // IDs must match InfoTable order (ID = InfoTable index + 1).
            // OPT_INPUT=1 and OPT_UNKNOWN=2 are always first.
            // The remaining IDs are sorted by pure option name using
            // StrCmpOptionName semantics (case-insensitive; longer wins when
            // one name is a prefix of another, so "target-cpu" < "target").
            enum ID {
                OPT_INVALID = 0,        // not an option ID
                OPT_INPUT,              //  1 - <input>
                OPT_UNKNOWN,            //  2 - <unknown>
                OPT_DEBUG,              //  3 - debug    (-debug)
                OPT_EMIT_AS,            //  4 - emit-as  (-emit-as)
                OPT_EMIT_BC,            //  5 - emit-bc  (-emit-bc)
                OPT_EMIT_LL,            //  6 - emit-ll  (-emit-ll)
                OPT_FTIME_REPORT,       //  7 - ftime-report  (-ftime-report)
                OPT_HEADER_GENERATOR,   //  8 - header   (-header)
                OPT_HELP,               //  9 - help     (-help)
                OPT_OUTPUT_LIB,         // 10 - lib      (-lib)
                OPT_LOG_FILE,           // 11 - log-file (-log-file)
                OPT_MC_MODEL,           // 12 - mcmodel  (-mcmodel)
                OPT_MTHREAD_MODEL,      // 13 - mthread-model (-mthread-model)
                OPT_NO_OUTPUT,          // 14 - no-output (-no-output)
                OPT_OUTPUT,             // 15 - o        (-o)
                OPT_PRINT_STATS,        // 16 - print-stats (-print-stats)
                OPT_STATS_FILE,         // 17 - stats-file (-stats-file)
                OPT_TARGET_CPU,         // 18 - target-cpu (--target-cpu) [before target]
                OPT_TARGET,             // 19 - target   (--target)
                OPT_VERSION,            // 20 - version  (-version) [before v]
                OPT_VERBOSE,            // 21 - v        (-v)
                OPT_WORKING_DIR,        // 22 - working-dir (-working-dir) [before w]
                OPT_NO_WARNING,         // 23 - w        (-w)
            };
        }

        const llvm::opt::OptTable &getDriverOptTable();
    }
}

#endif
