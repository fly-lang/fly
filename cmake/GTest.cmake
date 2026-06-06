include(FetchContent)

FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0)

# Modern approach - FetchContent_MakeAvailable handles everything
FetchContent_MakeAvailable(googletest)

# Redirect GTest/GMock archives to build/gtest/ instead of build/lib/
# (googletest's internal_utils.cmake hardcodes CMAKE_BINARY_DIR/lib).
set_target_properties(gtest gtest_main gmock gmock_main PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/gtest"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/gtest"
)

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
if (MSVC)
    message(STATUS "MSVC Build -> GTEST_LINKED_AS_SHARED_LIBRARY 1")
    set(GTEST_LINKED_AS_SHARED_LIBRARY 1)
endif()
