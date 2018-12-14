
include(CMakeParseArguments)

function(build_autogen)

    set(oneValueArgs

        EXECUTABLE_TOOL
        AUTOGEN_DIR 
        AUTOGEN_POSTFIX      
    )

    set(multiValueArgs 

        PROC_FILES
        OUT_AUTOGEN_FILES
        )

    cmake_parse_arguments(ARGS  "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Generation tool invocation command:
    #set( FOUNDATION_AUTOGEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/autogen")
    file (MAKE_DIRECTORY ARGS_AUTOGEN_DIR)

    set(AUTOGEN_FILES "")

    foreach (proc_file ${ARGS_PROC_FILES})

        get_filename_component(DIRECTORY_NAME ${proc_file} DIRECTORY)
        get_filename_component(BASE_NAME ${proc_file} NAME_WE)
        get_filename_component(EXTENSION ${proc_file} EXT)

        # make sure the path is absolute
        if (NOT IS_ABSOLUTE ${proc_file})
            set(DIRECTORY_NAME "${ARGS_FOUNDATION_AUTOGEN_DIR}/${DIRECTORY_NAME}")
        endif ()

        file(RELATIVE_PATH REL_AUTOGEN_FILE ${ARGS_AUTOGEN_DIR} "${DIRECTORY_NAME}/${BASE_NAME}_${ARGS_AUTOGEN_POSTFIX}.cpp" )
        set( AUTOGEN_FILE  "${ARGS_AUTOGEN_DIR}/${REL_AUTOGEN_FILE}" )

        # that's by default so it should not be necesssary
        set_source_files_properties(${AUTOGEN_FILE} PROPERTIES GENERATED TRUE)

        list(APPEND AUTOGEN_FILES ${AUTOGEN_FILE} )
    endforeach ()

    add_custom_command(
        OUTPUT ${AUTOGEN_FILES}
        COMMAND ${ARGS_EXECUTABLE_TOOL}   ${ARGS_AUTOGEN_DIR}  ${ARGS_PROC_FILES} 
        MAIN_DEPENDENCY ${PROC_FILES}
        COMMENT " .. Generating files with ${ARGS_EXECUTABLE_TOOL} "
    )

    set(${ARGS_OUT_AUTOGEN_FILES} "${AUTOGEN_FILES}" PARENT_SCOPE)

    message(warning "\n ARGS_OUT_AUTOGEN_FILES " ${ARGS_OUT_AUTOGEN_FILES} " .. " )

endfunction()
