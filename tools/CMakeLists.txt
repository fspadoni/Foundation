cmake_minimum_required(VERSION 3.2)

project(tools)

message(STATUS "\nConfiguring Project " ${PROJECT_NAME} " .. " )


set(SOURCE_FILES main.cpp)

message(STATUS "EXECUTABLE_OUTPUT_PATH " ${EXECUTABLE_OUTPUT_PATH} )
message(STATUS "CMAKE_CURRENT_BINARY_DIR " ${CMAKE_CURRENT_BINARY_DIR} )


set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")

include(DownloadProject)
download_project(PROJ                llvm
                 GIT_REPOSITORY      https://git.llvm.org/git/llvm.git/
                 GIT_TAG             release_60
                 PREFIX 			 "${CMAKE_BINARY_DIR}"
                 DOWNLOAD_DIR		 "${CMAKE_SOURCE_DIR}/third_party/llvm-download"
        		 SOURCE_DIR          "${CMAKE_SOURCE_DIR}/third_party/llvm"
        		 BINARY_DIR          "${CMAKE_BINARY_DIR}/llvm"
                 ${UPDATE_DISCONNECTED_IF_AVAILABLE}
)


download_project(PROJ                clang
                 GIT_REPOSITORY      https://git.llvm.org/git/clang.git/
                 GIT_TAG             release_60
                 PREFIX 			 "${CMAKE_BINARY_DIR}"
                 DOWNLOAD_DIR		 "${CMAKE_SOURCE_DIR}/third_party/clang-download"
        		 SOURCE_DIR          "${CMAKE_SOURCE_DIR}/third_party/llvm/tools/clang"
        		 BINARY_DIR          "${CMAKE_BINARY_DIR}/llvm/tools/clang"
                 ${UPDATE_DISCONNECTED_IF_AVAILABLE}
)


# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)


# don't build tests
set(LLVM_INCLUDE_TOOLS ON CACHE BOOL "" FORCE)
set(LLVM_BUILD_TOOLS ON CACHE BOOL "" FORCE)
set(LLVM_INCLUDE_TESTS OFF CACHE BOOL "" FORCE)
set(LLVM_INCLUDE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LLVM_INCLUDE_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(LLVM_APPEND_VC_REV OFF CACHE BOOL "" FORCE)

set(LLVM_INCLUDE_GO_TESTS OFF CACHE BOOL "" FORCE)
set(LLVM_INCLUDE_DOCS OFF CACHE BOOL "" FORCE)

set(LLVM_INCLUDE_UTILS ON CACHE BOOL "" FORCE)
# set(LLVM_BUILD_UTILS OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

# enable clang project
set(LLVM_ENABLE_PROJECTS "llvm/tools/clang" CACHE STRING "" FORCE)

set(CLANG_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(CLANG_ENABLE_STATIC_ANALYZER OFF CACHE BOOL "" FORCE)
set(CLANG_ENABLE_ARCMT OFF CACHE BOOL "" FORCE)


add_subdirectory(${llvm_SOURCE_DIR} ${llvm_BINARY_DIR})

# add_subdirectory(${clang_SOURCE_DIR} ${clang_BINARY_DIR})



# include_directories(${CLANG_INCLUDE_DIRS} ${LLVM_INCLUDE_DIRS})

message(STATUS "\nclang_SOURCE_DIR " ${clang_SOURCE_DIR} " .. " )
message(STATUS "\nllvm_SOURCE_DIR " ${llvm_SOURCE_DIR} " .. " )

message(STATUS "\nLLVM_EXTERNAL_CLANG_SOURCE_DIR " ${LLVM_EXTERNAL_CLANG_SOURCE_DIR} " .. " )


# Command from the LLVM CMake module
# add_llvm_executable(tools main.cpp)
add_executable(toolTest main.cpp)

# Minimum set of the required Clang libraries for the tool
target_link_libraries(toolTest ClangAST ClangBasic ClangDriver ClangFrontend ClangRewriteFrontend 
   ClangStaticAnalyzerFrontend ClangTooling)


