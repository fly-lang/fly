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
// String table: a single packed null-terminated string literal.
// Offset 0 MUST be the empty string (required by StringTable).
//
// Byte offsets (cumulative, each entry = string chars + '\0'):
//  0   : ""             (1 byte,  empty string required by StringTable)
//  1   : "-"            (2 bytes, next=3)
//  3   : "--"           (3 bytes, next=6)
//  6   : "<input>"      (8 bytes, next=14)
//  14  : "<unknown>"    (10 bytes, next=24)
//  24  : "-as"          (4 bytes, next=28)
//  28  : "-bc"          (4 bytes, next=32)
//  32  : "-debug"       (7 bytes, next=39)
//  39  : "-ftime-report"(14 bytes, next=53)
//  53  : "--help"       (7 bytes, next=60)
//  60  : "-H"           (3 bytes, next=63)
//  63  : "-lib"         (5 bytes, next=68)
//  68  : "-log-file"    (10 bytes, next=78)
//  78  : "-ll"          (4 bytes, next=82)
//  82  : "-mc-model"    (10 bytes, next=92)
//  92  : "-mthread-mode"(14 bytes, next=106)
//  106 : "-no"          (4 bytes, next=110)
//  110 : "-o"           (3 bytes, next=113)
//  113 : "-print-stats" (13 bytes, next=126)
//  126 : "-stats-file=" (13 bytes, next=139)
//  139 : "-target-cpu"  (12 bytes, next=151)
//  151 : "-target"      (8 bytes, next=159)
//  159 : "--version-short" (16 bytes, next=175)
//  175 : "-version-short"  (15 bytes, next=190)
//  190 : "--version"    (10 bytes, next=200)
//  200 : "-version"     (9 bytes, next=209)
//  209 : "-v"           (3 bytes, next=212)
//  212 : "-w"           (3 bytes, next=215)
//  215 : "-working-dir" (13 bytes, next=228)
// ---------------------------------------------------------------------------
static constexpr llvm::StringTable OptionStrTable(
    "\0"                 // 0
    "-\0"                // 1
    "--\0"               // 3
    "<input>\0"          // 6
    "<unknown>\0"        // 14
    "-as\0"              // 24
    "-bc\0"              // 28
    "-debug\0"           // 32
    "-ftime-report\0"    // 39
    "--help\0"           // 53
    "-H\0"               // 60
    "-lib\0"             // 63
    "-log-file\0"        // 68
    "-ll\0"              // 78
    "-mc-model\0"        // 82
    "-mthread-mode\0"    // 92
    "-no\0"              // 106
    "-o\0"               // 110
    "-print-stats\0"     // 113
    "-stats-file=\0"     // 126
    "-target-cpu\0"      // 139
    "-target\0"          // 151
    "--version-short\0"  // 159
    "-version-short\0"   // 175
    "--version\0"        // 190
    "-version\0"         // 200
    "-v\0"               // 209
    "-w\0"               // 212
    "-working-dir\0"     // 215
);

namespace {

// ---------------------------------------------------------------------------
// OptionPrefixesTable: encodes sets of prefixes as arrays of StringTable offsets.
//
// Each prefix set starts with its count, followed by that many offsets into
// OptionStrTable. The PrefixesOffset field in Info points to the count slot.
//
// Layout:
//   [0] = 0  (sentinel - PrefixesOffset=0 means "no prefix")
//   [1] = 1  (count=1, single "-" prefix)
//   [2] = 1  (offset of "-" in StrTable)
//   [3] = 2  (count=2, "--" first then "-"; first prefix used by getName())
//   [4] = 3  (offset of "--" in StrTable, first = canonical prefix for getName())
//   [5] = 1  (offset of "-" in StrTable)
//
// PrefixesOffset values used in InfoTable:
//   0 = no prefix  (INPUT, UNKNOWN)
//   1 = single "-" prefix
//   3 = "--" and "-" prefixes (canonical prefix is "--" so PrefixedNameOffset is "--name")
// ---------------------------------------------------------------------------
static constexpr llvm::StringTable::Offset OptionPrefixesTable[] = {
    {0},  // [0] sentinel (no-prefix)
    {1},  // [1] count=1
    {1},  // [2] offset of "-"
    {2},  // [3] count=2
    {3},  // [4] offset of "--" (first/canonical prefix)
    {1},  // [5] offset of "-"
};

// Helper macro for empty HelpTextsForVariants
#define NO_VARIANTS (std::array<std::pair<std::array<unsigned int, 2>, const char *>, 1>{{ {std::array<unsigned int, 2>{{0u, 0u}}, nullptr} }})

// ---------------------------------------------------------------------------
// Option info table
// Fields: PrefixesOffset, PrefixedNameOffset, HelpText, HelpTextsForVariants,
//         MetaVar, ID, Kind, Param, Flags, Visibility, GroupID, AliasID,
//         AliasArgs, Values
// ---------------------------------------------------------------------------
static const OptTable::Info InfoTable[] = {
    // Fields: PrefixesOffset, PrefixedNameOffset, HelpText, HelpTextsForVariants,
    //         MetaVar, ID, Kind, Param, Flags, Visibility, GroupID, AliasID,
    //         AliasArgs, Values
    {0, {6},   nullptr, NO_VARIANTS, nullptr, OPT_INPUT,          Option::InputClass,            0, 0, 0, 0, 0, nullptr, nullptr},
    {0, {14},  nullptr, NO_VARIANTS, nullptr, OPT_UNKNOWN,         Option::UnknownClass,          0, 0, 0, 0, 0, nullptr, nullptr},
    {1, {24},  "Produce assembly output .as file",            NO_VARIANTS, nullptr, OPT_EMIT_AS,       Option::FlagClass,             0, 0, 0, 0, 0, nullptr, nullptr},
    {1, {28},  "Produce bitecode output .bc file",            NO_VARIANTS, nullptr, OPT_EMIT_BC,       Option::FlagClass,             0, 0, 0, 0, 0, nullptr, nullptr},
    {1, {32},  "Print debug messages",                        NO_VARIANTS, nullptr, OPT_DEBUG,         Option::FlagClass,             0, 0, 0, 0, 0, nullptr, nullptr},
    {1, {39},  "Show timers for individual actions",          NO_VARIANTS, nullptr, OPT_FTIME_REPORT,  Option::FlagClass,             0, 0, 0, 0, 0, nullptr, nullptr},
    {3, {53},  "Display available options",                   NO_VARIANTS, nullptr, OPT_HELP,          Option::FlagClass,             0, 0, 0, 0, 0, nullptr, nullptr},
    {1, {60},  "Generate Header File",                        NO_VARIANTS, nullptr, OPT_HEADER_GENERATOR, Option::FlagClass,          0, 0, 0, 0, 0, nullptr, nullptr},
    {1, {63},  "Write output as library to <file>.lib",       NO_VARIANTS, nullptr, OPT_OUTPUT_LIB,    Option::JoinedOrSeparateClass, 0, 0, 0, 0, 0, nullptr, nullptr},
    {1, {68},  "Log diagnostics to <file>",                   NO_VARIANTS, nullptr, OPT_LOG_FILE,      Option::JoinedOrSeparateClass, 0, 0, 0, 0, 0, nullptr, nullptr},
    {1, {78},  "Produce LLVM formats output .ll file",        NO_VARIANTS, nullptr, OPT_EMIT_LL,       Option::FlagClass,             0, 0, 0, 0, 0, nullptr, nullptr},
    {1, {82},  "Produce different Memory Code Model",         NO_VARIANTS, nullptr, OPT_MC_MODEL,      Option::FlagClass,             0, 0, 0, 0, 0, nullptr, nullptr},
    {1, {92},  "Produce different Memory Thread Model",       NO_VARIANTS, nullptr, OPT_MTHREAD_MODEL, Option::FlagClass,             0, 0, 0, 0, 0, nullptr, nullptr},
    {1, {106}, "Produce no output",                           NO_VARIANTS, nullptr, OPT_NO_OUTPUT,     Option::FlagClass,             0, 0, 0, 0, 0, nullptr, nullptr},
    {1, {110}, "Write output to <file>",                      NO_VARIANTS, nullptr, OPT_OUTPUT,        Option::JoinedOrSeparateClass, 0, 0, 0, 0, 0, nullptr, nullptr},
    {1, {113}, "Print performance metrics and statistics",    NO_VARIANTS, nullptr, OPT_PRINT_STATS,   Option::FlagClass,             0, 0, 0, 0, 0, nullptr, nullptr},
    {1, {126}, "Filename to write statistics to",             NO_VARIANTS, nullptr, OPT_STATS_FILE,    Option::JoinedClass,           0, 0, 0, 0, 0, nullptr, nullptr},
    {1, {139}, "Generate code for the given CPU",             NO_VARIANTS, nullptr, OPT_TARGET_CPU,    Option::SeparateClass,         0, 0, 0, 0, 0, nullptr, nullptr},
    {1, {151}, "Generate code for the given target",          NO_VARIANTS, nullptr, OPT_TARGET,        Option::SeparateClass,         0, 0, 0, 0, 0, nullptr, nullptr},
    {3, {159}, "Print version number",                        NO_VARIANTS, nullptr, OPT_VERSION_SHORT, Option::FlagClass,             0, 0, 0, 0, 0, nullptr, nullptr},
    {3, {190}, "Print version information",                   NO_VARIANTS, nullptr, OPT_VERSION,       Option::FlagClass,             0, 0, 0, 0, 0, nullptr, nullptr},
    {1, {209}, "Show commands to run and use verbose output", NO_VARIANTS, nullptr, OPT_VERBOSE,       Option::FlagClass,             0, 0, 0, 0, 0, nullptr, nullptr},
    {1, {215}, "ResolveModule file paths relative to the specified directory", NO_VARIANTS, nullptr, OPT_WORKING_DIR, Option::JoinedOrSeparateClass, 0, 0, 0, 0, 0, nullptr, nullptr},
    {1, {212}, "Suppress all warnings",                       NO_VARIANTS, nullptr, OPT_NO_WARNING,    Option::FlagClass,             0, 0, 0, 0, 0, nullptr, nullptr},
};

#undef NO_VARIANTS

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
