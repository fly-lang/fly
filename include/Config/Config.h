#ifdef FLY_CONFIG_H
#error config.h can only be included once
#else
#define FLY_CONFIG_H

#include <string>

/* Bug report URL. */
#define FLY_BUG_REPORT_URL "https://github.com/fly-lang/fly/issues"
#define FLY_VERSION "0.10.0"

const std::string FLY_SOURCE_DIR = "/home/marco/Projects/flylang/fly";
const std::string FLY_LLVM_BINARY_DIR = "/home/marco/Projects/flylang/fly/cmake-build-release/llvm-project";
const std::string FLY_LLVM_DIR = "/home/marco/Projects/flylang/fly/cmake-build-release/llvm";
const std::string FLY_RUNTIME_LIB_DIR = "/home/marco/Projects/flylang/fly/cmake-build-release/lib";
#define FLY_LLVM_VERSION_MAJOR 20


#endif
