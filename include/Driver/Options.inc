//===--------------------------------------------------------------------------------------------------------------===//
// include/Driver/Options.inc - Option Parsing Definitions
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

/////////
// Prefixes

#ifdef PREFIX
#define COMMA ,
PREFIX(prefix_0, {nullptr})
PREFIX(prefix_1, {"-" COMMA nullptr})
PREFIX(prefix_4, {"-" COMMA "--" COMMA nullptr})
#undef COMMA
#endif // PREFIX

/**
 * All Accepted KIND:
 *  Group,
 *  Input,
 *  Unknown
 *  Flag,
 *  Joined,
 *  Values,
 *  Separate,
 *  RemainingArgs,
 *  RemainingArgsJoined,
 *  CommaJoined,
 *  MultiArg,
 *  JoinedOrSeparate,
 *  JoinedAndSeparate
 */


// Fields:
// OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, ALIASARGS, FLAGS, PARAM, HELPTEXT, METAVAR, VALUES)

// IMPORTANT: Respect Alphabetically order by ID!!

#ifdef OPTION
OPTION(prefix_0, "<input>", INPUT, Input, INVALID, INVALID, nullptr, 0, 0, nullptr, nullptr, nullptr)
OPTION(prefix_0, "<unknown>", UNKNOWN, Unknown, INVALID, INVALID, nullptr, 0, 0, nullptr, nullptr, nullptr)
OPTION(prefix_4, "help", HELP, Flag, INVALID, INVALID, nullptr, CoreOption, 0,
       "Display available options", nullptr, nullptr)
OPTION(prefix_1, "o", OUTPUT, JoinedOrSeparate, INVALID, INVALID, nullptr, CoreOption, 0,
       "Write output to <file>", "<file>", nullptr)
OPTION(prefix_1, "v", VERBOSE, Flag, INVALID, INVALID, nullptr, CoreOption, 0,
       "Show commands to run and use verbose output", nullptr, nullptr)
OPTION(prefix_4, "version", VERSION, Flag, INVALID, INVALID, nullptr, CoreOption, 0,
       "Print version information", nullptr, nullptr)
#endif // OPTION
