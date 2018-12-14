cmake_minimum_required(VERSION 3.2)

project(foundation)

message(STATUS "\nConfiguring Project " ${PROJECT_NAME} " .. " )

message(STATUS "\nLLVM_SOURCE_DIR " ${LLVM_SOURCE_DIR} " .. " )
message(STATUS "\nllvm_SOURCE_DIR " ${llvm_SOURCE_DIR} " .. " )

set_property(GLOBAL PROPERTY USE_FOLDERS ON)


set ( FOUNDATION_HEADER_FILES
	src/TaskScheduler.h
	src/Task.h
 	src/Task.inl
 	
)

set ( FOUNDATION_PROC_FILES
 	src/TaskScheduler.h
 	src/Task.h
)

set ( FOUNDATION_SOURCE_FILES
 	src/TaskScheduler.cpp
 	src/Task.cpp
)


if(MACOSX OR APPLE)
	file( GLOB FOUNDATION_PLATFORM_INCLUDE  "src/platform/mac/*.h" )
elseif(LINUX)
	file( GLOB FOUNDATION_PLATFORM_INCLUDE  "src/platform/linux/*.h" )
elseif(WINDOWS)
	file( GLOB FOUNDATION_PLATFORM_INCLUDE  "src/platform/win32/*.h" )
elseif(ANDROID)
	file( GLOB FOUNDATION_PLATFORM_INCLUDE  "src/platform/android/*.h" )
endif()


# Generation tool invocation command:
include(AutoGenerate)

set(FOUNDATION_AUTOGEN_FILES "")

build_autogen(
	EXECUTABLE_TOOL		toolTest
    AUTOGEN_DIR 		"${CMAKE_CURRENT_BINARY_DIR}/autogen"
    AUTOGEN_POSTFIX     "autogen"
    PROC_FILES 			${FOUNDATION_PROC_FILES}
    
    OUT_AUTOGEN_FILES 	FOUNDATION_AUTOGEN_FILES
	)

message(warning "\n FOUNDATION_AUTOGEN_FILES " ${FOUNDATION_AUTOGEN_FILES} " .. " )

source_group("autogen" FILES ${FOUNDATION_AUTOGEN_FILES} )


include_directories(src)
include_directories(src/platform)
include_directories(.)
include_directories(${FOUNDATION_AUTOGEN_DIR})


if(BUILD_FOUNDATION_SHARED)
  add_library(foundation SHARED
    ${FOUNDATION_SOURCE_FILES}
    ${FOUNDATION_HEADER_FILES}
    ${FOUNDATION_PLATFORM_INCLUDE}
    ${FOUNDATION_AUTOGEN_FILES}
  )
  # install(TARGETS foundation RUNTIME DESTINATION lib LIBRARY DESTINATION lib)
  message(STATUS "build shared library" )
endif(BUILD_FOUNDATION_SHARED)

if(BUILD_FOUNDATION_STATIC)
  add_library(foundation_static STATIC
    ${FOUNDATION_SOURCE_FILES}
    ${FOUNDATION_HEADER_FILES}
    ${FOUNDATION_PLATFORM_INCLUDE}
    ${FOUNDATION_AUTOGEN_FILES}
  )

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