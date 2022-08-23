include(FetchContent)

FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main)
FetchContent_GetProperties(googletest)
FetchContent_MakeAvailable(googletest)

if (POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)

set(GOOGLETEST_VERSION 1.11.0)
if (CMAKE_VERSION VERSION_GREATER "3.0.2")
    if(NOT CYGWIN AND NOT MSYS AND NOT ${CMAKE_SYSTEM_NAME} STREQUAL QNX)
        set(CMAKE_CXX_EXTENSIONS OFF)
    endif()
endif()

enable_testing()

include(CMakeDependentOption)
include(GNUInstallDirs)

#Note that googlemock target already builds googletest
option(BUILD_GMOCK "Builds the googlemock subproject" ON)
option(INSTALL_GTEST "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)" ON)

if(BUILD_GMOCK)
    add_subdirectory( googlemock )
else()
    add_subdirectory( googletest )
endif()

#
#set(GTEST_INCLUDE_DIRS ${source_dir}/googletest/include)
#set(GMOCK_INCLUDE_DIRS ${source_dir}/googlemock/include)
## The cloning of the above repo doesn't happen until make, however if the dir doesn't
## exist, INTERFACE_INCLUDE_DIRECTORIES will throw an error.
## To make it work, we just create the directory now during config.
#file(MAKE_DIRECTORY ${GTEST_INCLUDE_DIRS})
#file(MAKE_DIRECTORY ${GMOCK_INCLUDE_DIRS})
#
#ExternalProject_Get_Property(googletest binary_dir)
#set(GTEST_LIBRARY_PATH ${binary_dir}/googlemock/gtest/${CMAKE_FIND_LIBRARY_PREFIXES}gtest.a)
#set(GTEST_LIBRARY gtest)
#add_library(${GTEST_LIBRARY} UNKNOWN IMPORTED)
#set_target_properties(${GTEST_LIBRARY} PROPERTIES
#        "IMPORTED_LOCATION" "${GTEST_LIBRARY_PATH}"
#        "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
#        "INTERFACE_INCLUDE_DIRECTORIES" "${GTEST_INCLUDE_DIRS}")
#add_dependencies(${GTEST_LIBRARY} googletest)
#
#set(GTEST_MAIN_LIBRARY_PATH ${binary_dir}/googlemock/gtest/${CMAKE_FIND_LIBRARY_PREFIXES}gtest_main.a)
#set(GTEST_MAIN_LIBRARY gtest_main)
#add_library(${GTEST_MAIN_LIBRARY} UNKNOWN IMPORTED)
#set_target_properties(${GTEST_MAIN_LIBRARY} PROPERTIES
#        "IMPORTED_LOCATION" "${GTEST_MAIN_LIBRARY_PATH}"
#        "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
#        "INTERFACE_INCLUDE_DIRECTORIES" "${GTEST_INCLUDE_DIRS}")
#add_dependencies(${GTEST_MAIN_LIBRARY} googletest)
#
#set(GMOCK_LIBRARY_PATH ${binary_dir}/googlemock/${CMAKE_FIND_LIBRARY_PREFIXES}gmock.a)
#set(GMOCK_LIBRARY gmock)
#add_library(${GMOCK_LIBRARY} UNKNOWN IMPORTED)
#set_target_properties(${GMOCK_LIBRARY} PROPERTIES
#        "IMPORTED_LOCATION" "${GMOCK_LIBRARY_PATH}"
#        "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
#        "INTERFACE_INCLUDE_DIRECTORIES" "${GMOCK_INCLUDE_DIRS}")
#add_dependencies(${GMOCK_LIBRARY} googletest)
#
#set(GMOCK_MAIN_LIBRARY_PATH ${binary_dir}/googlemock/${CMAKE_FIND_LIBRARY_PREFIXES}gmock_main.a)
#set(GMOCK_MAIN_LIBRARY gmock_main)
#add_library(${GMOCK_MAIN_LIBRARY} UNKNOWN IMPORTED)
#set_target_properties(${GMOCK_MAIN_LIBRARY} PROPERTIES
#        "IMPORTED_LOCATION" "${GMOCK_MAIN_LIBRARY_PATH}"
#        "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
#        "INTERFACE_INCLUDE_DIRECTORIES" "${GMOCK_INCLUDE_DIRS}")
#add_dependencies(${GMOCK_MAIN_LIBRARY} ${GTEST_LIBRARY})
