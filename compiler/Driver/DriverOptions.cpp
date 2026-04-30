//===--------------------------------------------------------------------------------------------------------------===//
// src/Driver/DriverOptions.cpp - Driver Options Table
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Driver/DriverOptions.h"
#include "llvm/ADT/StringTable.h"
#include "llvm/Option/OptTable.h"
#include "llvm/Option/Option.h"
#include <array>
#include <utility>

using namespace fly::driver;
using namespace fly::driver::options;
using namespace llvm::opt;

// ---------------------------------------------------------------------------
// String table – single packed null-terminated string literal.
// Offset 0 MUST be the empty string (StringTable requirement).
//
// The InfoTable is sorted by pure option name (PrefixedName minus the
// canonical prefix) using LLVM's StrCmpOptionName ordering, which is
// case-insensitive with '\0' sorting LAST (so longer options sort before
// any shorter option that is a prefix of them, e.g. "target-cpu" < "target").
//
// Byte offsets (each entry = string chars + '\0'):
//   0  : ""               (1  byte,  next=1)
//   1  : "-"              (2  bytes, next=3)
//   3  : "--"             (3  bytes, next=6)
//   6  : "<input>"        (8  bytes, next=14)
//  14  : "<unknown>"      (10 bytes, next=24)
//  24  : "-debug"         (7  bytes, next=31)
//  31  : "-emit-as"       (9  bytes, next=40)
//  40  : "-emit-bc"       (9  bytes, next=49)
//  49  : "-emit-ll"       (9  bytes, next=58)
//  58  : "-ftime-report"  (14 bytes, next=72)
//  72  : "-header"        (8  bytes, next=80)
//  80  : "-help"          (6  bytes, next=86)
//  86  : "-lib"           (5  bytes, next=91)
//  91  : "-log-file"      (10 bytes, next=101)
// 101  : "-mcmodel"       (9  bytes, next=110)
// 110  : "-mthread-model" (15 bytes, next=125)
// 125  : "-no-output"     (11 bytes, next=136)
// 136  : "-o"             (3  bytes, next=139)
// 139  : "-print-stats"   (13 bytes, next=152)
// 152  : "-stats-file"    (12 bytes, next=164)
// 164  : "--target-cpu"   (13 bytes, next=177)
// 177  : "--target"       (9  bytes, next=186)
// 186  : "-version"       (9  bytes, next=195)
// 195  : "-v"             (3  bytes, next=198)
// 198  : "-working-dir"   (13 bytes, next=211)
// 211  : "-w"             (3  bytes, next=214)
// ---------------------------------------------------------------------------
static constexpr llvm::StringTable OptionStrTable(
    "\0"                    //   0 – empty (required)
    "-\0"                   //   1
    "--\0"                  //   3
    "<input>\0"             //   6
    "<unknown>\0"           //  14
    "-debug\0"              //  24
    "-emit-as\0"            //  31
    "-emit-bc\0"            //  40
    "-emit-ll\0"            //  49
    "-ftime-report\0"       //  58
    "-header\0"             //  72
    "-help\0"               //  80
    "-lib\0"                //  86
    "-log-file\0"           //  91
    "-mcmodel\0"            // 101
    "-mthread-model\0"      // 110
    "-no-output\0"          // 125
    "-o\0"                  // 136
    "-print-stats\0"        // 139
    "-stats-file\0"         // 152
    "--target-cpu\0"        // 164
    "--target\0"            // 177
    "-version\0"            // 186
    "-v\0"                  // 195
    "-working-dir\0"        // 198
    "-w\0"                  // 211
);

namespace {

// ---------------------------------------------------------------------------
// OptionPrefixesTable – prefix sets encoded as arrays of StringTable offsets.
//
// Each set starts with its count followed by that many offsets into StrTable.
// The PrefixesOffset field in Info points to the count slot.
//
// Layout:
//   [0] = 0  sentinel (PrefixesOffset=0 ⇒ no prefix, used for INPUT/UNKNOWN)
//   [1] = 1  count=1
//   [2] = 1  offset of "-"              → PrefixesOffset=1 ⇒ single-dash only
//   [3] = 2  count=2
//   [4] = 3  offset of "--" (canonical) → PrefixesOffset=3 ⇒ "--" and "-" both accepted
//   [5] = 1  offset of "-"
// ---------------------------------------------------------------------------
static constexpr llvm::StringTable::Offset OptionPrefixesTable[] = {
    {0},  // [0] sentinel
    {1},  // [1] count=1
    {1},  // [2] offset of "-"
    {2},  // [3] count=2
    {3},  // [4] offset of "--" (canonical prefix)
    {1},  // [5] offset of "-"
};

#define NO_VARIANTS (std::array<std::pair<std::array<unsigned int, 2>, const char *>, 1>{{ {std::array<unsigned int, 2>{{0u, 0u}}, nullptr} }})

// All Fly driver options are visible in every driver context.
#define VIS (~0u)

// ---------------------------------------------------------------------------
// InfoTable – entries MUST be sorted by pure option name via LLVM's
// StrCmpOptionName (case-insensitive; '\0' sorts last so longer option names
// precede any shorter name they extend: "target-cpu" < "target").
//
// OPT_INPUT (index 0) and OPT_UNKNOWN (index 1) are always first.
// Each entry's ID must equal its 1-based position (ID = index + 1).
//
// Fields: PrefixesOffset, PrefixedNameOffset, HelpText, HelpTextsForVariants,
//         MetaVar, ID, Kind, Param, Flags, Visibility, GroupID, AliasID,
//         AliasArgs, Values
// ---------------------------------------------------------------------------
static const OptTable::Info InfoTable[] = {
    // ── INPUT / UNKNOWN (always first) ──────────────────────────────────────
    {0, {6},   nullptr, NO_VARIANTS, nullptr, OPT_INPUT,            Option::InputClass,            0, 0, VIS, 0, 0, nullptr, nullptr},
    {0, {14},  nullptr, NO_VARIANTS, nullptr, OPT_UNKNOWN,          Option::UnknownClass,          0, 0, VIS, 0, 0, nullptr, nullptr},
    // ── sorted by pure name ─────────────────────────────────────────────────
    {1, {24},  "Print debug messages",                               NO_VARIANTS, nullptr, OPT_DEBUG,            Option::FlagClass,             0, 0, VIS, 0, 0, nullptr, nullptr},
    {1, {31},  "Produce assembly output",                            NO_VARIANTS, nullptr, OPT_EMIT_AS,          Option::FlagClass,             0, 0, VIS, 0, 0, nullptr, nullptr},
    {1, {40},  "Produce bitcode output",                             NO_VARIANTS, nullptr, OPT_EMIT_BC,          Option::FlagClass,             0, 0, VIS, 0, 0, nullptr, nullptr},
    {1, {49},  "Produce LLVM IR output",                             NO_VARIANTS, nullptr, OPT_EMIT_LL,          Option::FlagClass,             0, 0, VIS, 0, 0, nullptr, nullptr},
    {1, {58},  "Show timers for individual actions",                 NO_VARIANTS, nullptr, OPT_FTIME_REPORT,     Option::FlagClass,             0, 0, VIS, 0, 0, nullptr, nullptr},
    {1, {72},  "Generate header file",                               NO_VARIANTS, nullptr, OPT_HEADER_GENERATOR, Option::FlagClass,             0, 0, VIS, 0, 0, nullptr, nullptr},
    {1, {80},  "Display available options",                          NO_VARIANTS, nullptr, OPT_HELP,             Option::FlagClass,             0, 0, VIS, 0, 0, nullptr, nullptr},
    {1, {86},  "Write output as library to <file>.a",                NO_VARIANTS, nullptr, OPT_OUTPUT_LIB,       Option::JoinedOrSeparateClass, 0, 0, VIS, 0, 0, nullptr, nullptr},
    {1, {91},  "Log diagnostics to <file>",                          NO_VARIANTS, nullptr, OPT_LOG_FILE,         Option::JoinedOrSeparateClass, 0, 0, VIS, 0, 0, nullptr, nullptr},
    {1, {101}, "Set memory code model",                              NO_VARIANTS, nullptr, OPT_MC_MODEL,         Option::JoinedOrSeparateClass, 0, 0, VIS, 0, 0, nullptr, nullptr},
    {1, {110}, "Set memory thread model",                            NO_VARIANTS, nullptr, OPT_MTHREAD_MODEL,    Option::JoinedOrSeparateClass, 0, 0, VIS, 0, 0, nullptr, nullptr},
    {1, {125}, "Produce no output",                                  NO_VARIANTS, nullptr, OPT_NO_OUTPUT,        Option::FlagClass,             0, 0, VIS, 0, 0, nullptr, nullptr},
    {1, {136}, "Write output to <file>",                             NO_VARIANTS, nullptr, OPT_OUTPUT,           Option::JoinedOrSeparateClass, 0, 0, VIS, 0, 0, nullptr, nullptr},
    {1, {139}, "Print performance metrics and statistics",           NO_VARIANTS, nullptr, OPT_PRINT_STATS,      Option::FlagClass,             0, 0, VIS, 0, 0, nullptr, nullptr},
    {1, {152}, "Filename to write statistics to",                    NO_VARIANTS, nullptr, OPT_STATS_FILE,       Option::JoinedOrSeparateClass, 0, 0, VIS, 0, 0, nullptr, nullptr},
    {3, {164}, "Generate code for the given CPU",                    NO_VARIANTS, nullptr, OPT_TARGET_CPU,       Option::SeparateClass,         0, 0, VIS, 0, 0, nullptr, nullptr},
    {3, {177}, "Generate code for the given target",                 NO_VARIANTS, nullptr, OPT_TARGET,           Option::SeparateClass,         0, 0, VIS, 0, 0, nullptr, nullptr},
    {1, {186}, "Print version information",                          NO_VARIANTS, nullptr, OPT_VERSION,          Option::FlagClass,             0, 0, VIS, 0, 0, nullptr, nullptr},
    {1, {195}, "Show commands to run and use verbose output",        NO_VARIANTS, nullptr, OPT_VERBOSE,          Option::FlagClass,             0, 0, VIS, 0, 0, nullptr, nullptr},
    {1, {198}, "Resolve file paths relative to the specified directory", NO_VARIANTS, nullptr, OPT_WORKING_DIR,  Option::JoinedOrSeparateClass, 0, 0, VIS, 0, 0, nullptr, nullptr},
    {1, {211}, "Suppress all warnings",                              NO_VARIANTS, nullptr, OPT_NO_WARNING,       Option::FlagClass,             0, 0, VIS, 0, 0, nullptr, nullptr},
};

#undef NO_VARIANTS
#undef VIS

class DriverOptTable : public GenericOptTable {
public:
    DriverOptTable()
        : GenericOptTable(OptionStrTable,
                          llvm::ArrayRef<llvm::StringTable::Offset>(OptionPrefixesTable),
                          InfoTable) {}
};

} // anonymous namespace

const llvm::opt::OptTable &fly::driver::getDriverOptTable() {
    static DriverOptTable *Opt = new DriverOptTable();
    return *Opt;
}
