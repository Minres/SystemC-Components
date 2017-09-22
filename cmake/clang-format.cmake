# additional target to perform clang-format run, requires clang-format

# get all project files
file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp *.h)
foreach (SOURCE_FILE ${ALL_SOURCE_FILES})
    foreach(PROJECT_TRDPARTY_DIR  ${PROJECT_3PARTY_DIRS})
        string(FIND ${SOURCE_FILE} ${PROJECT_TRDPARTY_DIR} PROJECT_EXTERNAL_DIR_FOUND)
        if (NOT ${PROJECT_EXTERNAL_DIR_FOUND} EQUAL -1)
            #message( STATUS "Skipping ${SOURCE_FILE} for clang-format." )
            list(REMOVE_ITEM ALL_SOURCE_FILES ${SOURCE_FILE})
        endif ()
    endforeach()
endforeach ()

add_custom_target(
        clangformat
        COMMAND clang-format
        -i
        ${ALL_SOURCE_FILES}
)
