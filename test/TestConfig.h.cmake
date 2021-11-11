#ifdef FLY_CONFIGTEST_H
#error ConfigTest.h can only be included once
#else
#define FLY_CONFIGTEST_H
#include <string>

const std::string FLY_TEST_BUILD_PATH = "${CMAKE_CURRENT_BINARY_DIR}";
const std::string FLY_TEST_SRC_PATH = "${CMAKE_CURRENT_SOURCE_DIR}/src";

#endif