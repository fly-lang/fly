# Add Google Test dependency
find_package(GTest REQUIRED)
if (DEFINED GTEST_FOUND)
    message(STATUS "Found GTest")
    include_directories(${GTEST_INCLUDE_DIRS})
else ()
    message(FATAL_ERROR "GTest not found")
endif ()

add_subdirectory(Lex)