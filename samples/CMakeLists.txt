cmake_minimum_required(VERSION 3.2)

project(test)

message(STATUS "\nConfiguring Project " ${PROJECT_NAME} " .. " )



set(SOURCE_FILES main.cpp)

add_executable(schedulerTest ${SOURCE_FILES})

message(STATUS "EXECUTABLE_OUTPUT_PATH " ${EXECUTABLE_OUTPUT_PATH} )
message(STATUS "CMAKE_CURRENT_BINARY_DIR " ${CMAKE_CURRENT_BINARY_DIR} )




