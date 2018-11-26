cmake_minimum_required(VERSION 3.2)

project(framework)

message(STATUS "\nConfiguring Project " ${PROJECT_NAME} " .. " )

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

message( STATUS "CMAKE_C_COMPILER = ${CMAKE_C_COMPILER}" )
message( STATUS "CMAKE_CXX_COMPILER = ${CMAKE_CXX_COMPILER}" )

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

option(BUILD_FOUNDATION_SHARED "Build and install the shared library" OFF)
option(BUILD_FOUNDATION_STATIC "Build as static library" ON)
# option(INSTALL_FOUNDATION_STATIC "Install the static library" OFF)

# Add third party libs
# add_subdirectory(third_party)


add_subdirectory(foundation)

# add_subdirectory(tools)
# add_subdirectory(samples)

# add_subdirectory(tests)

# find_library(ICONV iconv)
# target_link_libraries(${PROJECT_NAME} PUBLIC ${ICONV})
