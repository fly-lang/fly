/*===-----------------------------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Option Parsing Definitions                                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

/////////
// Prefixes

#ifdef PREFIX
#define COMMA ,
PREFIX(prefix_0, {nullptr})
PREFIX(prefix_1, {"-" COMMA nullptr})
PREFIX(prefix_2, {"-" COMMA "--" COMMA nullptr})
#undef COMMA
#endif // PREFIX

// Fields:
// OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, ALIASARGS, FLAGS, PARAM, HELPTEXT, METAVAR, VALUES)

#ifdef OPTION
OPTION(prefix_1, "i", input_file, Flag, INVALID, INVALID, nullptr, CoreOption, 0, "Read input source", nullptr, nullptr)
OPTION(prefix_1, "o", output_file, Flag, INVALID, INVALID, nullptr, CoreOption, 0, "Write object to file (default file.o)", nullptr, nullptr)
OPTION(prefix_2, "v", version, Flag, INVALID, version, nullptr, CoreOption, 0, "Print version information", nullptr, nullptr)
OPTION(prefix_2, "h", help, Flag, INVALID, help, nullptr, CoreOption, 0, "Print help commands", nullptr, nullptr)
#endif // OPTION
