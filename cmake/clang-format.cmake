# additional target to perform clang-format run, requires clang-format

set(CLANG_FORMAT_CXX_FILE_EXTENSIONS ${CLANG_FORMAT_CXX_FILE_EXTENSIONS} *.cpp *.h *.cxx *.hxx *.hpp *.cc *.ipp)
file(GLOB_RECURSE ALL_SOURCE_FILES ${CLANG_FORMAT_CXX_FILE_EXTENSIONS})

# Don't include some common build folders
set(CLANG_FORMAT_EXCLUDE_PATTERNS ${CLANG_FORMAT_EXCLUDE_PATTERNS} "/CMakeFiles/" "cmake")

# get all project files file
foreach (SOURCE_FILE ${ALL_SOURCE_FILES}) 
    foreach (EXCLUDE_PATTERN ${CLANG_FORMAT_EXCLUDE_PATTERNS})
        string(FIND ${SOURCE_FILE} ${EXCLUDE_PATTERN} EXCLUDE_FOUND) 
        if (NOT ${EXCLUDE_FOUND} EQUAL -1) 
            list(REMOVE_ITEM ALL_SOURCE_FILES ${SOURCE_FILE})
        endif () 
    endforeach ()
endforeach ()

set(FORMAT_TARGET_NAME format)
if(NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	set(FORMAT_TARGET_NAME format-${PROJECT_NAME})
endif()

add_custom_target(${FORMAT_TARGET_NAME}
    COMMENT "Running clang-format to change files"
    COMMAND ${CLANG_FORMAT_BIN} -style=file -i ${ALL_SOURCE_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)


add_custom_target(${FORMAT_TARGET_NAME}-check
    COMMENT "Checking clang-format changes"
    # Use ! to negate the result for correct output
    COMMAND !
    ${CLANG_FORMAT_BIN}
    -style=file
    -output-replacements-xml
    ${ALL_SOURCE_FILES}
    | grep -q "replacement offset" 
)