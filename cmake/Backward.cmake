include(FetchContent)

FetchContent_Declare(backward
        GIT_REPOSITORY https://github.com/bombela/backward-cpp
        GIT_TAG v1.6)
FetchContent_GetProperties(backward)
if(NOT backward_POPULATED)
    FetchContent_Populate(backward)
    add_subdirectory(${backward_SOURCE_DIR} ${backward_BINARY_DIR})
endif()

FetchContent_MakeAvailable(backward)
