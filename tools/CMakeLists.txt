cmake_minimum_required(VERSION 3.2)

project(tools)

message(STATUS "\nConfiguring Project " ${PROJECT_NAME} " .. " )


set(SOURCE_FILES main.cpp)

message(STATUS "EXECUTABLE_OUTPUT_PATH " ${EXECUTABLE_OUTPUT_PATH} )
message(STATUS "CMAKE_CURRENT_BINARY_DIR " ${CMAKE_CURRENT_BINARY_DIR} )


set(LLVM_INSTALL_PREFIX $ENV{LLVM_INSTALL_PREFIX} CACHE PATH "Path to LLVM installation root directory")
set(LLVM_DIR "${LLVM_INSTALL_PREFIX}/lib/cmake/llvm" )
message(STATUS "LLVM_DIR " ${LLVM_DIR} )

list (APPEND CMAKE_MODULE_PATH ${LLVM_INSTALL_PREFIX}/lib/cmake/llvm)
list (APPEND CMAKE_MODULE_PATH ${LLVM_INSTALL_PREFIX}/lib/cmake/clang)

# CMake modules from the LLVM and Clang installation
include(AddLLVM)
include(ClangConfig)

include_directories(${CLANG_INCLUDE_DIRS} ${LLVM_INCLUDE_DIRS})

# Command from the LLVM CMake module
add_llvm_executable(tools main.cpp)

# Minimum set of the required Clang libraries for the tool
target_link_libraries(tools ClangAST ClangBasic ClangDriver ClangFrontend ClangRewriteFrontend 
   ClangStaticAnalyzerFrontend ClangTooling)
