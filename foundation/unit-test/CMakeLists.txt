cmake_minimum_required(VERSION 3.2)

project(foundationUnitTest)

message(STATUS "\nConfiguring Project " ${PROJECT_NAME} " .. " )

message(STATUS "CMAKE_SOURCE_DIR "  ${CMAKE_SOURCE_DIR}  "  .." ) 

# include(CTest)

# set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")

# include(DownloadProject)
# download_project(PROJ                googletest
#                  GIT_REPOSITORY      https://github.com/google/googletest.git
#                  GIT_TAG             master
#                  PREFIX 			 "${CMAKE_BINARY_DIR}"
#                  DOWNLOAD_DIR		 "${CMAKE_SOURCE_DIR}/third_party/googletest-download"
#         		 SOURCE_DIR          "${CMAKE_SOURCE_DIR}/third_party/googletest-src"
#         		 BINARY_DIR          "${CMAKE_BINARY_DIR}/googletest"
#                  ${UPDATE_DISCONNECTED_IF_AVAILABLE}
# )

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})

# When using CMake 2.8.11 or later, header path dependencies
# are automatically added to the gtest and gmock targets.
# For earlier CMake versions, we have to explicitly add the
# required directories to the header search path ourselves.
# if (CMAKE_VERSION VERSION_LESS 2.8.11)
#     include_directories("${gtest_SOURCE_DIR}/include"
#                         "${gmock_SOURCE_DIR}/include")
# endif()

# Trivial example using gtest and gmock
add_executable(example example.cpp)
target_link_libraries(example gtest gmock_main)
# add_test(NAME example_test COMMAND example)