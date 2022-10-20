include(FetchContent)

option(ABSL_PROPAGATE_CXX_STD
        "Use CMake C++ standard meta features (e.g. cxx_std_14) that propagate to targets that link to Abseil"
        ON)

FetchContent_Declare(abseil
        GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
        GIT_TAG master)
FetchContent_GetProperties(abseil)
if(NOT abseil_POPULATED)
    FetchContent_Populate(abseil)
    add_subdirectory(${abseil_SOURCE_DIR} ${abseil_BINARY_DIR})
endif()

FetchContent_MakeAvailable(abseil)
