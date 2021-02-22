//===----------------------------------------------------------------------===//
// Driver/DriverOptions.cpp - Driver Options Table
//
// Part of the Fly Project, under the Apache License v2.0
// See https://flylang.org/LICENSE.txt for license information.
// Thank you to LLVM Project https://llvm.org/
//
//===----------------------------------------------------------------------===//

#include "Driver/Options.h"
#include "llvm/Option/OptTable.h"
#include "llvm/Option/Option.h"
#include <cassert>

using namespace fly::driver;
using namespace fly::driver::options;
using namespace llvm::opt;

#define PREFIX(NAME, VALUE) static const char *const NAME[] = VALUE;
#include "Driver/Options.inc"
#undef PREFIX

static const OptTable::Info InfoTable[] = {
#define OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, ALIASARGS, FLAGS, PARAM,  \
               HELPTEXT, METAVAR, VALUES)                                      \
  {PREFIX, NAME,  HELPTEXT,    METAVAR,     OPT_##ID,  Option::KIND##Class,    \
   PARAM,  FLAGS, OPT_##GROUP, OPT_##ALIAS, ALIASARGS, VALUES},
#include "Driver/Options.inc"
#undef OPTION
};

namespace {

class DriverOptTable : public OptTable {
public:
  DriverOptTable()
    : OptTable(InfoTable) {}
};

}

const llvm::opt::OptTable &fly::driver::getDriverOptTable() {
  static const DriverOptTable *Table = []() {
    auto Result = std::make_unique<DriverOptTable>();
    // Options.inc is included in DriverOptions.cpp, and calls OptTable's
    // addValues function.
    // Opt is a variable used in the code fragment in Options.inc.
    OptTable &Opt = *Result;
#define OPTTABLE_ARG_INIT
#include "Driver/Options.inc"
#undef OPTTABLE_ARG_INIT
    return Result.release();
  }();
  return *Table;
}
