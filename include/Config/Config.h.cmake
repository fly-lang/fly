#ifdef FLY_CONFIG_H
#error config.h can only be included once
#else
#define FLY_CONFIG_H

/* Bug report URL. */
#define FLY_BUG_REPORT_URL "${FLY_BUG_REPORT_URL}"
#define FLY_VERSION "${FLY_VERSION}"

const std::string FLY_SOURCE_DIR = "${FLY_SOURCE_DIR}";
const std::string FLY_LLVM_BINARY_DIR = "${FLY_LLVM_BINARY_DIR}";
const std::string GCC_LIB_PATH = "${GCC_LIB_PATH}";


#endif
