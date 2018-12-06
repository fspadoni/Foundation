cmake_minimum_required(VERSION 3.2)

project(foundation)

message(STATUS "\nConfiguring Project " ${PROJECT_NAME} " .. " )

message(STATUS "\nLLVM_SOURCE_DIR " ${LLVM_SOURCE_DIR} " .. " )
message(STATUS "\nllvm_SOURCE_DIR " ${llvm_SOURCE_DIR} " .. " )




file(GLOB foundation_source_files "src/*.cpp")
file(GLOB foundation_header_files "src/*.h" "src/*.inl")

if(MACOSX OR APPLE)
	file( GLOB foundation_platform_include  "src/platform/mac/*.h" )
elseif(LINUX)
	file( GLOB foundation_platform_include  "src/platform/linux/*.h" )
elseif(WINDOWS)
	file( GLOB foundation_platform_include  "src/platform/win32/*.h" )
elseif(ANDROID)
	file( GLOB foundation_platform_include  "src/platform/android/*.h" )
endif()


# Generation tool invocation command:
set (CODEGEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/codegen)
file (MAKE_DIRECTORY ${CODEGEN_DIR}/generated)

set (foundation_source_files_generated ${CODEGEN_DIR}/generated/*.cpp)
set (foundation_header_files_generated ${CODEGEN_DIR}/generated/*.h)

set( CODEGEN_BIN_NAME "tool")


# add_custom_command(OUTPUT ${GENERATED_FILES}
#    COMMAND ${CODEGEN_BIN_NAME} -o ${GENERATED_FILES}
#        ${CMAKE_CURRENT_SOURCE_DIR}/generated_test.h -- Clang-cl -std=c++14 -x c++ ${CMAKE_CXX_FLAGS}
#    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/test_enums.h
#    COMMENT "Generating files for ${CMAKE_CURRENT_SOURCE_DIR}/test_enums.h"
#    )



include_directories(src)
include_directories(src/platform)
include_directories(.)
include_directories(generated)

if(BUILD_FOUNDATION_SHARED)
  add_library(foundation SHARED
    ${foundation_source_files}
    ${foundation_header_files}
    ${foundation_platform_include}
  )
  # install(TARGETS foundation RUNTIME DESTINATION lib LIBRARY DESTINATION lib)
  message(STATUS "build shared library" )
endif(BUILD_FOUNDATION_SHARED)

if(BUILD_FOUNDATION_STATIC)
  add_library(foundation_static STATIC
    ${foundation_source_files}
    ${foundation_header_files}
    ${foundation_platform_include}
  )
  # Sets chipmunk_static to output "libchipmunk.a" not "libchipmunk_static.a"
  set_target_properties(foundation_static PROPERTIES OUTPUT_NAME foundation)
  # if(INSTALL_FOUNDATION_STATIC)
    # install(TARGETS foundation_static ARCHIVE DESTINATION ${install_dir} )
  # endif(INSTALL_FOUNDATION_STATIC)
  message(STATUS  "build static library" )
endif(BUILD_FOUNDATION_STATIC)

if(BUILD_FOUNDATION_SHARED OR INSTALL_FOUNDATION_STATIC)
  # FIXME: change to PUBLIC_HEADER to allow building frameworks
  install(FILES ${foundation_include_files} DESTINATION include/foundation)
endif(BUILD_FOUNDATION_SHARED OR INSTALL_FOUNDATION_STATIC)


message(STATUS "EXECUTABLE_OUTPUT_PATH " ${EXECUTABLE_OUTPUT_PATH} )
message(STATUS "CMAKE_CURRENT_BINARY_DIR " ${CMAKE_CURRENT_BINARY_DIR} )



add_subdirectory(unit-test)