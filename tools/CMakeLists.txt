cmake_minimum_required(VERSION 3.2)

project(foundationTool)

message(STATUS "\nConfiguring Project " ${PROJECT_NAME} " .. " )


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

set(LLVM_INCLUDE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LLVM_INCLUDE_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(LLVM_APPEND_VC_REV OFF CACHE BOOL "" FORCE)

set(LLVM_INCLUDE_GO_TESTS OFF CACHE BOOL "" FORCE)
set(LLVM_INCLUDE_DOCS OFF CACHE BOOL "" FORCE)

# add gtest
set(LLVM_INCLUDE_UTILS ON CACHE BOOL "" FORCE)
set(LLVM_INCLUDE_TESTS ON CACHE BOOL "" FORCE)
# set(LLVM_BUILD_UTILS OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

# enable clang project
set(LLVM_ENABLE_PROJECTS "llvm/tools/clang" CACHE STRING "" FORCE)

set(CLANG_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(CLANG_ENABLE_STATIC_ANALYZER ON CACHE BOOL "" FORCE)
set(CLANG_ENABLE_ARCMT OFF CACHE BOOL "" FORCE)

set(CLANG_PLUGIN_SUPPORT OFF CACHE BOOL "" FORCE)


add_subdirectory(${llvm_SOURCE_DIR} ${llvm_BINARY_DIR})

# add_subdirectory(${clang_SOURCE_DIR} ${clang_BINARY_DIR})


set(SOURCE_FILES main.cpp)




# They are used as destination of target generators.
set(LLVM_RUNTIME_OUTPUT_INTDIR ${llvm_BINARY_DIR}/${CMAKE_CFG_INTDIR}/bin)
set(LLVM_LIBRARY_OUTPUT_INTDIR ${llvm_BINARY_DIR}/${CMAKE_CFG_INTDIR}/lib${LLVM_LIBDIR_SUFFIX})
if(WIN32 OR CYGWIN)
  # DLL platform -- put DLLs into bin.
  set(LLVM_SHLIB_OUTPUT_INTDIR ${LLVM_RUNTIME_OUTPUT_INTDIR})
else()
  set(LLVM_SHLIB_OUTPUT_INTDIR ${LLVM_LIBRARY_OUTPUT_INTDIR})
endif()

# Each of them corresponds to llvm-config's.
set(LLVM_TOOLS_BINARY_DIR ${LLVM_RUNTIME_OUTPUT_INTDIR}) # --bindir
set(LLVM_LIBRARY_DIR      ${LLVM_LIBRARY_OUTPUT_INTDIR}) # --libdir
set(LLVM_MAIN_SRC_DIR     ${llvm_SOURCE_DIR}  ) # --src-root
set(LLVM_MAIN_INCLUDE_DIR ${LLVM_MAIN_SRC_DIR}/include ) # --includedir
set(LLVM_BINARY_DIR       ${llvm_BINARY_DIR}  ) # --prefix

# Note: LLVM_CMAKE_PATH does not include generated files
set(LLVM_CMAKE_PATH ${LLVM_MAIN_SRC_DIR}/cmake/modules)
set(LLVM_EXAMPLES_BINARY_DIR ${LLVM_BINARY_DIR}/examples)
set(LLVM_INCLUDE_DIR ${LLVM_BINARY_DIR}/include)



# include_directories

# List of all targets to be built by default:
set(LLVM_TARGETS
  AArch64
  AMDGPU
  ARM
  BPF
  Hexagon
  Lanai
  Mips
  MSP430
  NVPTX
  PowerPC
  Sparc
  SystemZ
  X86
  XCore
  )

if( LLVM_TARGETS_TO_BUILD STREQUAL "all" )
  set( LLVM_TARGETS_TO_BUILD ${LLVM_TARGETS} )
endif()

message(STATUS "\n  foundationToollibs " ${LLVM_TARGETS}  )
set( LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS}
  Option
  Support
  )


# Command from the LLVM CMake module
add_llvm_executable(foundationTool main.cpp)


set( LLVM_LIB_DEPENDENCIES  
	# ${LLVM_ALL_TARGETS}
	LLVMSupport 
	)

set( CLANG_LIB_DEPENDENCIES  

	clangAST 
	clangBasic 
	clangDriver 
	clangFrontend 
	clangRewriteFrontend 
	clangStaticAnalyzerFrontend 
	clangTooling 
	)


foreach(llvmlib ${LLVM_LIB_DEPENDENCIES})
	get_target_property(include_dirs  ${llvmlib}  INCLUDE_DIRECTORIES)
	foreach(dir ${include_dirs})
		# message(STATUS "\n  foundationTool include dirs " ${dir}  )
		list(APPEND INCLUDE_DIRS "${dir}" )
	endforeach()
endforeach()

foreach(clanglib ${CLANG_LIB_DEPENDENCIES})
	get_target_property(include_dirs  ${clanglib}  INCLUDE_DIRECTORIES)
	foreach(dir ${include_dirs})
		list(APPEND INCLUDE_DIRS "${dir}" )
	endforeach()
endforeach()

list(REMOVE_DUPLICATES INCLUDE_DIRS)

message(STATUS "\n  foundationTool include dirs " ${INCLUDE_DIRS}  )

# target_include_directories(foundationTool PRIVATE ${INCLUDE_DIRS} )


include_directories( ${LLVM_INCLUDE_DIR} ${LLVM_MAIN_INCLUDE_DIR} ${LLVM_BINARY_DIR})
# include_directories(BEFORE
#     ${CMAKE_CURRENT_BINARY_DIR}
#     ${CMAKE_CURRENT_SOURCE_DIR})

include_directories( ${clang_SOURCE_DIR}/include ${clang_BINARY_DIR}/include  )

# Minimum set of the required Clang libraries for the tool
target_link_libraries(foundationTool PRIVATE 
	clangAST 
	clangBasic 
	clangDriver 
	clangFrontend 
	clangRewriteFrontend 
	clangStaticAnalyzerFrontend 
	clangTooling  )

