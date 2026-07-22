# function to finde recursively files matching INCLUDE_PATTERNS and not matching EXCLUDE_PATTERNS
function(find_sources result_var)
    set(_options)
    set(_one_value_args BASE_DIR)
    set(_multi_value_args EXCLUDE_PATTERNS INCLUDE_PATTERNS)
    cmake_parse_arguments(FIND_SOURCES "${_options}" "${_one_value_args}" "${_multi_value_args}" ${ARGN})

    if(NOT FIND_SOURCES_BASE_DIR)
        message(FATAL_ERROR "find_sources requires BASE_DIR")
    endif()

    if(NOT FIND_SOURCES_INCLUDE_PATTERNS)
        message(FATAL_ERROR "find_sources requires at least one INCLUDE_PATTERNS entry")
    endif()

    if(NOT IS_DIRECTORY "${FIND_SOURCES_BASE_DIR}")
        message(FATAL_ERROR "find_sources base directory does not exist: ${FIND_SOURCES_BASE_DIR}")
    endif()
    # message(STATUS "Entering ${FIND_SOURCES_BASE_DIR}")
    set(_exclude_list ${FIND_SOURCES_EXCLUDE_PATTERNS})
    set(_include_patterns ${FIND_SOURCES_INCLUDE_PATTERNS})
    set(_result)
    set(_excluded_entries)
    set(_top_level_matches)
    set(_glob_options LIST_DIRECTORIES true)
    set(_file_glob_options LIST_DIRECTORIES false)

    if(NOT DEFINED CMAKE_SCRIPT_MODE_FILE)
        list(PREPEND _glob_options CONFIGURE_DEPENDS)
        list(PREPEND _file_glob_options CONFIGURE_DEPENDS)
    endif()

    file(GLOB _entries ${_glob_options} "${FIND_SOURCES_BASE_DIR}/*")

    foreach(_pattern IN LISTS _exclude_list)
        file(GLOB _pattern_excludes ${_glob_options} "${FIND_SOURCES_BASE_DIR}/${_pattern}")
        list(APPEND _excluded_entries ${_pattern_excludes})
    endforeach()

    foreach(_pattern IN LISTS _include_patterns)
        file(GLOB _pattern_matches ${_file_glob_options} "${FIND_SOURCES_BASE_DIR}/${_pattern}")
        list(APPEND _top_level_matches ${_pattern_matches})
    endforeach()

    list(REMOVE_DUPLICATES _excluded_entries)
    list(REMOVE_DUPLICATES _top_level_matches)

    foreach(_entry IN LISTS _entries)
        if(_entry IN_LIST _excluded_entries)
            continue()
        endif()

        if(IS_DIRECTORY "${_entry}")
                find_sources(
                    _recursive_matches
                    BASE_DIR "${_entry}"
                    EXCLUDE_PATTERNS ${_exclude_list}
                    INCLUDE_PATTERNS ${_include_patterns}
                )
                list(APPEND _result ${_recursive_matches})
        elseif(_entry IN_LIST _top_level_matches)
            list(APPEND _result "${_entry}")
        endif()
    endforeach()

    list(REMOVE_DUPLICATES _result)
    list(SORT _result)
    set(${result_var} ${_result} PARENT_SCOPE)
endfunction()

# additional target to perform clang-format run, requires clang-format
set(CLANG_FORMAT_CXX_FILE_EXTENSIONS ${CLANG_FORMAT_CXX_FILE_EXTENSIONS} *.cpp *.h *.cxx *.hxx *.hpp *.cc *.ipp)
message(STATUS "Enumerating files for clang-format")
# get all project files file
find_sources(
    ALL_SOURCE_FILES
    BASE_DIR "."
    EXCLUDE_PATTERNS ${CLANG_FORMAT_EXCLUDE_PATTERNS}
    INCLUDE_PATTERNS ${CLANG_FORMAT_CXX_FILE_EXTENSIONS}
)
message(STATUS "Done enumerating files for clang-format")

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
    COMMAND ${CLANG_FORMAT_BIN} -style=file --dry-run -Werror ${ALL_SOURCE_FILES}
)