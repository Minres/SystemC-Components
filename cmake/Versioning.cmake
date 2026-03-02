include_guard(GLOBAL)

function(scc_configure_version)
    set(options
        USE_GIT
        RELEASE_ENFORCE_TAG
        RELEASE_FAIL_DIRTY
        INSTALL_HEADER
        REPRODUCIBLE
    )

    set(oneValueArgs
        TARGET
        NAMESPACE
        OUTPUT_HEADER
        ABI_VERSION
        TAG_PREFIX
    )

    cmake_parse_arguments(SCC "${options}" "${oneValueArgs}" "" ${ARGN})

    if(NOT PROJECT_VERSION)
        message(FATAL_ERROR "project(VERSION ...) is required")
    endif()

    if(NOT SCC_TARGET OR NOT SCC_NAMESPACE OR NOT SCC_OUTPUT_HEADER)
        message(FATAL_ERROR "TARGET, NAMESPACE and OUTPUT_HEADER required")
    endif()

    set(VERSION_FULL "${PROJECT_VERSION}")
    set(VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
    set(VERSION_MINOR "${PROJECT_VERSION_MINOR}")

    # ------------------------------------------------------------
    # ABI version (explicit only)
    # ------------------------------------------------------------
    if(SCC_ABI_VERSION)
        set(ABI_VERSION "${SCC_ABI_VERSION}")
    else()
        set(ABI_VERSION "0") # default safe ABI
    endif()

    # ------------------------------------------------------------
    # Git metadata
    # ------------------------------------------------------------
    set(GIT_HASH "unknown")
    set(GIT_DESCRIBE "")
    set(GIT_DIRTY OFF)

    if(SCC_USE_GIT)
        find_package(Git QUIET)
        if(GIT_FOUND)
            execute_process(
                COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE GIT_HASH
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET
            )

            execute_process(
                COMMAND ${GIT_EXECUTABLE} describe --tags --dirty --always
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE GIT_DESCRIBE
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET
            )

            string(FIND "${GIT_DESCRIBE}" "-dirty" DIRTY_POS)
            if(NOT DIRTY_POS EQUAL -1)
                set(GIT_DIRTY ON)
            endif()
        endif()
    endif()

    # ------------------------------------------------------------
    # Tag enforcement (exact match, optional prefix)
    # ------------------------------------------------------------
    if(SCC_RELEASE_ENFORCE_TAG)
        if(SCC_TAG_PREFIX)
            set(EXPECTED_TAG "${SCC_TAG_PREFIX}${PROJECT_VERSION}")
        else()
            set(EXPECTED_TAG "${PROJECT_VERSION}")
        endif()

        if(NOT GIT_DESCRIBE STREQUAL "${EXPECTED_TAG}")
            message(FATAL_ERROR
                "Release requires git tag '${EXPECTED_TAG}'")
        endif()
    endif()

    if(SCC_RELEASE_FAIL_DIRTY AND GIT_DIRTY)
        message(FATAL_ERROR
            "Release build from dirty working tree not allowed")
    endif()

    # ------------------------------------------------------------
    # Reproducible timestamp support
    # ------------------------------------------------------------
    if(SCC_REPRODUCIBLE AND DEFINED ENV{SOURCE_DATE_EPOCH})
        math(EXPR _epoch "$ENV{SOURCE_DATE_EPOCH}")
        string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%dT%H:%M:%SZ"
            UTC ${_epoch})
    else()
        string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%dT%H:%M:%SZ" UTC)
    endif()

    # ------------------------------------------------------------
    # Build metadata
    # ------------------------------------------------------------
    if(CMAKE_BUILD_TYPE)
        set(BUILD_TYPE "${CMAKE_BUILD_TYPE}")
    else()
        set(BUILD_TYPE "multi-config")
    endif()

    set(COMPILER_ID "${CMAKE_CXX_COMPILER_ID}")
    set(COMPILER_VERSION "${CMAKE_CXX_COMPILER_VERSION}")

    # ------------------------------------------------------------
    # Generate header
    # ------------------------------------------------------------
    get_filename_component(_outdir ${SCC_OUTPUT_HEADER} DIRECTORY)
    file(MAKE_DIRECTORY ${_outdir})

    configure_file(
        ${CMAKE_CURRENT_LIST_DIR}/src/scc_ver.h.in
        ${SCC_OUTPUT_HEADER}
        @ONLY
    )

    target_include_directories(${SCC_TARGET}
        INTERFACE
        $<BUILD_INTERFACE:${_outdir}>
        $<INSTALL_INTERFACE:include>
    )

    # ------------------------------------------------------------
    # Shared library ABI / SOVERSION
    # ------------------------------------------------------------
    get_target_property(_type ${SCC_TARGET} TYPE)
    if(_type STREQUAL "SHARED_LIBRARY")
        set_target_properties(${SCC_TARGET} PROPERTIES
            VERSION ${VERSION_FULL}
            SOVERSION ${ABI_VERSION}
        )
    endif()

    # ------------------------------------------------------------
    # CPack propagation
    # ------------------------------------------------------------
    set(CPACK_PACKAGE_VERSION "${VERSION_FULL}" PARENT_SCOPE)

    # ------------------------------------------------------------
    # Optional install
    # ------------------------------------------------------------
    if(SCC_INSTALL_HEADER)
        install(FILES ${SCC_OUTPUT_HEADER}
            DESTINATION include)
    endif()

    message(STATUS "Configured ${SCC_NAMESPACE} version ${VERSION_FULL}")
    message(STATUS "  ABI version: ${ABI_VERSION}")
    message(STATUS "  Git: ${GIT_HASH}")
    message(STATUS "  Dirty: ${GIT_DIRTY}")
endfunction()
