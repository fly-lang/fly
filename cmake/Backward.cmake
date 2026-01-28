include(FetchContent)

# Use a recent commit that supports CMake 3.5+
# Alternatively, you can use GIT_TAG v1.6.1 or later if a new release is available
FetchContent_Declare(backward
        GIT_REPOSITORY https://github.com/bombela/backward-cpp
        GIT_TAG master)  # Use master branch for latest CMake compatibility

# Modern approach - FetchContent_MakeAvailable handles everything
FetchContent_MakeAvailable(backward)
