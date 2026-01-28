include(FetchContent)

FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1)

# Modern approach - FetchContent_MakeAvailable handles everything
FetchContent_MakeAvailable(googletest)

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
if (MSVC)
    message(STATUS "MSVC Build -> GTEST_LINKED_AS_SHARED_LIBRARY 1")
    set(GTEST_LINKED_AS_SHARED_LIBRARY 1)
endif()
