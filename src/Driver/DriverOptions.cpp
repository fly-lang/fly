//===--------------------------------------------------------------------------------------------------------------===//
// src/Driver/DriverOptions.cpp - Driver Options Table
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Driver/DriverOptions.h"
#include "llvm/Option/OptTable.h"
#include "llvm/Option/Option.h"

using namespace fly::driver;
using namespace fly::driver::options;
using namespace llvm::opt;

static const char *const prefix_0[] = {nullptr};
static const char *const prefix_1[] = {"-", nullptr};
static const char *const prefix_2[] = {"-", "--", nullptr};

static const OptTable::Info InfoTable[] = {
    //{PREFIX, NAME, HELPTEXT, METAVAR, OPT_##ID,  Option::KIND##Class, PARAM,  FLAGS, OPT_##GROUP, OPT_##ALIAS, ALIASARGS, VALUES}
    {prefix_0, "<input>", nullptr, nullptr, OPT_INPUT, Option::OptionClass::InputClass, 0, 0, 0, 0, nullptr, nullptr},
    {prefix_0, "<unknown>", nullptr, nullptr, OPT_UNKNOWN, Option::OptionClass::UnknownClass, 0, 0, 0, 0, nullptr, nullptr},
    {prefix_1, "as", "Produce assembly output .as file", nullptr, OPT_EMIT_AS, Option::OptionClass::FlagClass, 0, 0, 0, 0, nullptr, nullptr},
    {prefix_1, "bc", "Produce bitecode output .bc file", nullptr, OPT_EMIT_BC, Option::OptionClass::FlagClass, 0, 0, 0, 0, nullptr, nullptr},
    {prefix_1, "debug", "Print debug messages", nullptr, OPT_DEBUG, Option::OptionClass::FlagClass, 0, 0, 0, 0, nullptr, nullptr},
    {prefix_1, "ftime-report", "Show timers for individual actions", nullptr, OPT_FTIME_REPORT, Option::OptionClass::FlagClass, 0, 0, 0, 0, nullptr, nullptr},
    {prefix_2, "help", "Display available options", nullptr, OPT_HELP, Option::OptionClass::FlagClass, 0, 0, 0, 0, nullptr, nullptr},
    {prefix_1, "H", "Generate Header File", nullptr, OPT_HEADER_GENERATOR, Option::OptionClass::FlagClass, 0, 0, 0, 0, nullptr, nullptr},
    {prefix_1, "lib", "Write output as library to <file>.lib", nullptr, OPT_OUTPUT_LIB, Option::OptionClass::JoinedOrSeparateClass, 0, 0, 0, 0, nullptr, nullptr},
    {prefix_1, "log-file", "Log diagnostics to <file>", nullptr, OPT_LOG_FILE, Option::OptionClass::JoinedOrSeparateClass, 0, 0, 0, 0, nullptr, nullptr},
    {prefix_1, "ll", "Produce LLVM formats output .ll file", nullptr, OPT_EMIT_LL, Option::OptionClass::FlagClass, 0, 0, 0, 0, nullptr, nullptr},
    {prefix_1, "mc-model", "Produce different Memory Code Model", nullptr, OPT_MC_MODEL, Option::OptionClass::FlagClass, 0, 0, 0, 0, nullptr, nullptr},
    {prefix_1, "mthread-mode", "Produce different Memory Thread Model", nullptr, OPT_MTHREAD_MODEL, Option::OptionClass::FlagClass, 0, 0, 0, 0, nullptr, nullptr},
    {prefix_1, "no", "Produce no output", nullptr, OPT_NO_OUTPUT, Option::OptionClass::FlagClass, 0, 0, 0, 0, nullptr, nullptr},
    {prefix_1, "o", "Write output to <file>", nullptr, OPT_OUTPUT, Option::OptionClass::JoinedOrSeparateClass, 0, 0, 0, 0, nullptr, nullptr},
    {prefix_1, "print-stats", "Print performance metrics and statistics", nullptr, OPT_PRINT_STATS, Option::OptionClass::FlagClass, 0, 0, 0, 0, nullptr, nullptr},
    {prefix_1, "stats-file=", "Filename to write statistics to", nullptr, OPT_STATS_FILE, Option::OptionClass::JoinedClass, 0, 0, 0, 0, nullptr, nullptr},
    {prefix_1, "target-cpu", "Generate code for the given CPU", nullptr, OPT_TARGET_CPU, Option::OptionClass::SeparateClass, 0, 0, 0, 0, nullptr, nullptr},
    {prefix_1, "target", "Generate code for the given target", nullptr, OPT_VERSION_SHORT, Option::OptionClass::SeparateClass, 0, 0, 0, 0, nullptr, nullptr},
    {prefix_2, "version-short", "Print version number", nullptr, OPT_VERSION_SHORT, Option::OptionClass::FlagClass, 0, 0, 0, 0, nullptr, nullptr},
    {prefix_2, "version", "Print version information", nullptr, OPT_VERSION, Option::OptionClass::FlagClass, 0, 0, 0, 0, nullptr, nullptr},
    {prefix_1, "v", "Show commands to run and use verbose output", nullptr, OPT_VERBOSE, Option::OptionClass::FlagClass, 0, 0, 0, 0, nullptr, nullptr},
    {prefix_1, "working-dir", "Resolve file paths relative to the specified directory", nullptr, OPT_WORKING_DIR, Option::OptionClass::JoinedOrSeparateClass, 0, 0, 0, 0, nullptr, nullptr},
    {prefix_1, "w", "Suppress all warnings", nullptr, OPT_NO_WARNING, Option::OptionClass::FlagClass, 0, 0, 0, 0, nullptr, nullptr},
};

namespace {

class DriverOptTable : public OptTable {
public:
  DriverOptTable() : OptTable(InfoTable) {}
};

}

const llvm::opt::OptTable &fly::driver::getDriverOptTable() {
  DriverOptTable *Opt = new DriverOptTable();
  return *Opt;
}
