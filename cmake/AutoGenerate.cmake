
include(CMakeParseArguments)

function(prepend VAR PREFIX)
  set( ${VAR} ${PREFIX}${VAR}  PARENT_SCOPE)
  message(warning "\n VAR  " ${VAR}  )
endfunction(prepend)


function(build_autogen)

    set(oneValueArgs

        EXECUTABLE_TOOL
        AUTOGEN_DIR 
        AUTOGEN_POSTFIX      
    )

    set(multiValueArgs 

        PROC_FILES
        INCLUDE_DIRS
        OUT_AUTOGEN_FILES
        )

    cmake_parse_arguments(ARGS  "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Generation tool invocation command:
    #set( FOUNDATION_AUTOGEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/autogen")
    #file (MAKE_DIRECTORY ARGS_AUTOGEN_DIR)

    set(AUTOGEN_FILES "")

    foreach (proc_file ${ARGS_PROC_FILES})

        get_filename_component(DIRECTORY_NAME ${proc_file} DIRECTORY)
        get_filename_component(BASE_NAME ${proc_file} NAME_WE)
        get_filename_component(EXTENSION ${proc_file} EXT)

        message(warning "\n DIRECTORY_NAME " ${DIRECTORY_NAME} " .. " )
        
        set( FULL_AUTOGEN_DIR ${ARGS_AUTOGEN_DIR}/${DIRECTORY_NAME}  )
        file (MAKE_DIRECTORY ${FULL_AUTOGEN_DIR} )
        message(warning "\n FULL_AUTOGEN_DIR " ${FULL_AUTOGEN_DIR} " .. " )

        # make sure the path is absolute
        # if (NOT IS_ABSOLUTE ${proc_file})
        #     set(DIRECTORY_NAME "${ARGS_FOUNDATION_AUTOGEN_DIR}/${DIRECTORY_NAME}")
        #     message(warning "\n NOT IS_ABSOLUTE " ${proc_file} " .. " )
        # endif ()

        # file(RELATIVE_PATH REL_AUTOGEN_FILE ${ARGS_AUTOGEN_DIR} "${DIRECTORY_NAME}/${BASE_NAME}_${ARGS_AUTOGEN_POSTFIX}.cpp" )

        # message(warning "\n REL_AUTOGEN_FILE " ${REL_AUTOGEN_FILE} " .. " )

        set( AUTOGEN_FILE  "${FULL_AUTOGEN_DIR}/${BASE_NAME}_${ARGS_AUTOGEN_POSTFIX}.cpp" )

        message(warning "\n AUTOGEN_FILE " ${AUTOGEN_FILE} " .. " )

        # that's by default so it should not be necesssary
        set_source_files_properties(${AUTOGEN_FILE} PROPERTIES GENERATED TRUE)

        list(APPEND AUTOGEN_FILES ${AUTOGEN_FILE} )
    endforeach ()

    #list(TRANSFORM ARGS_INCLUDE_DIRS PREPEND "-I" )

    foreach (include_dir ${ARGS_INCLUDE_DIRS})
        # set( ${include_dir}  "-I${include_dir}")  
        # message(warning "\n include_dir  " ${include_dir}  )
        list(APPEND COMMAND_LINE_INCLUDE_DIRS "-I${include_dir}" )
        message(warning "\n COMMAND_LINE_INCLUDE_DIRS  " ${COMMAND_LINE_INCLUDE_DIRS}  )
    endforeach ()

    # SET(${ARGS_INCLUDE_DIRS} "${COMMAND_LINE_INCLUDE_DIRS}" PARENT_SCOPE)

    # foreach(dir ${ARGS_INCLUDE_DIRS})
    #     list(APPEND INCLUDE_DIRS -I"${dir}" PARENT_SCOPE )
    #     message( "\n dir  " ${dir} " .. " )
    # endforeach()
    # SET(${ARGS_INCLUDE_DIRS} "${INCLUDE_DIRS}" )
    # message( "\n ARGS_INCLUDE_DIRS  " ${ARGS_INCLUDE_DIRS} " .. " )

    add_custom_command(
        OUTPUT ${AUTOGEN_FILES}
        COMMAND ${ARGS_EXECUTABLE_TOOL}   ${ARGS_AUTOGEN_DIR}  ${ARGS_PROC_FILES} ${COMMAND_LINE_INCLUDE_DIRS}
        MAIN_DEPENDENCY ${PROC_FILES}
        COMMENT " .. Generating files with ${ARGS_EXECUTABLE_TOOL} "
    )

    set(${ARGS_OUT_AUTOGEN_FILES} "${AUTOGEN_FILES}" PARENT_SCOPE)

    message(warning "\n ARGS_OUT_AUTOGEN_FILES " ${ARGS_OUT_AUTOGEN_FILES} " .. " )

endfunction()
