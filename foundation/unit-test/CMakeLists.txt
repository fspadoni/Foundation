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

message(STATUS "\nLLVM_SOURCE_DIR " ${LLVM_SOURCE_DIR} " .. " )
include_directories(${LLVM_SOURCE_DIR}/utils/unittest/googletest/include)
include_directories(${LLVM_SOURCE_DIR}/utils/unittest/googlemock/include)

add_definitions( -DGTEST_NO_LLVM_RAW_OSTREAM=1 )
# set(LLVM_LINK_COMPONENTS  Support )

# Trivial example using gtest and gmock
add_executable(example1 example.cpp)
target_link_libraries(example1 foundation_static gtest gtest_main)
# target_link_libraries(${test_name} PRIVATE gtest_main gtest ${LLVM_PTHREAD_LIB})


set ( FOUNDATION_ASYNC_HEADER_FILES
	async/TaskSchedulerTestTasks.h
)
set ( FOUNDATION_ASYNC_SOURCE_FILES
	async/TaskScheduler_Tests.cpp
	async/TaskSchedulerTestTasks.cpp
)

include_directories(.)

add_executable(FoundationAsyncTest ${FOUNDATION_ASYNC_HEADER_FILES} ${FOUNDATION_ASYNC_SOURCE_FILES} )
target_link_libraries(FoundationAsyncTest foundation_static gtest gtest_main)
