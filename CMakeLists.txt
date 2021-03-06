cmake_minimum_required(VERSION 3.11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake) # project specific cmake dir

project(scc VERSION 1.0.0 LANGUAGES CXX C)

# find_package(Doxygen)

option(BUILD_SCC_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" OFF)

option(BUILD_STAND_ALONE "Do a standalone build and do not rely on a parent project providing needed libraries" FALSE)

include(Common)
include(GNUInstallDirs)
if(BUILD_STAND_ALONE)
	option(ENABLE_CLANG_TIDY "Add clang-tidy automatically to builds" OFF)
	include(Conan)
	setup_conan(TARGETS)
endif()

if(BUILD_SCC_DOCUMENTATION)
find_package(Doxygen)
    if(NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "Doxygen is needed to build the documentation.")
    endif()
	
	set(DOXYGEN_PROJECT_BRIEF "SystemC components library")
	set(DOXYGEN_USE_MDFILE_AS_MAINPAGE  README.md)
	# exclude json code
	set(DOXYGEN_EXCLUDE_PATTERNS  */json/*)
	# doxygen settings can be set here, prefixed with "DOXYGEN_"
	set(DOXYGEN_SOURCE_BROWSER YES)
	set(DOXYGEN_EXTRACT_ALL NO)
	set(DOXYGEN_EXTRACT_PRIVATE NO)
	set(DOXYGEN_SEARCHENGINE NO)
	set(DOXYGEN_GENERATE_HTML YES)
	set(DOXYGEN_GENERATE_MAN YES)
	set(DOXYGEN_GENERATE_LATEX NO)
	set(DOXYGEN_OUTPUT_DIRECTORY "${scc_BINARY_DIR}/scc")
	# this target will only be built if specifically asked to.
	# run "make scc-docs" to create the doxygen documentation
	doxygen_add_docs(
	    scc-docs
	    ${PROJECT_SOURCE_DIR}/incl ${PROJECT_SOURCE_DIR}/axi_chi/axi  ${PROJECT_SOURCE_DIR}/axi_chi/chi  ${PROJECT_SOURCE_DIR}/README.md
	    ALL COMMENT "Generate API-documents for SCC."
	)
    install(DIRECTORY ${scc_BINARY_DIR}/scc DESTINATION ${CMAKE_INSTALL_DOCDIR}/)
endif()


###############################################################################
# build the SCC
###############################################################################

set(CLANG_FORMAT_EXCLUDE_PATTERNS "/third_party/" "sqlite3")
find_package(ClangFormat)
if (ENABLE_CLANG_TIDY)
    find_program (CLANG_TIDY_EXE NAMES "clang-tidy" PATHS /usr/bin )
    if (CLANG_TIDY_EXE)
        message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
        set(CLANG_TIDY_CHECKS "-*,modernize-*")
        set(DO_CLANG_TIDY "${CLANG_TIDY_EXE};-checks=${CLANG_TIDY_CHECKS};-header-filter='${CMAKE_SOURCE_DIR}/*';-fix"
            CACHE STRING "" FORCE)
        #set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-checks=${CLANG_TIDY_CHECKS};-header-filter='${CMAKE_SOURCE_DIR}/*';-fix"
        #    CACHE STRING "" FORCE)
    else()
        message(AUTHOR_WARNING "clang-tidy not found!")
        set(CMAKE_CXX_CLANG_TIDY "" CACHE STRING "" FORCE) # delete it
    endif()
endif()

find_package(Boost REQUIRED COMPONENTS system) # header only libraries must not be added herefind_package(ZLIB REQUIRED)
include(CheckSymbolExists)
# Check for function getenv()
check_symbol_exists(getenv "stdlib.h" HAVE_GETENV)

# check which version of spdlog to use.
if(TARGET spdlog::spdlog)
    set(SPDLOG_TARGET spdlog::spdlog)
elseif(TARGET CONAN_PKG::spdlog)
    set(SPDLOG_TARGET CONAN_PKG::spdlog)
else()
    add_library(spdlog_local INTERFACE IMPORTED)
    set_property(TARGET spdlog_local PROPERTY INTERFACE_COMPILE_DEFINITIONS SPDLOG_HEADER_ONLY)
    set_property(TARGET spdlog_local PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
    set(SPDLOG_TARGET spdlog_local)
    message("Using built-in version of spdlog")
endif()

# check which version of fmt to use
if(TARGET fmt::fmt)
    set(FMT_TARGET fmt::fmt)
elseif(TARGET CONAN_PKG::fmt)
    set(FMT_TARGET CONAN_PKG::fmt)
else()
    add_library(fmt_local INTERFACE IMPORTED)
    set_property(TARGET fmt_local PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/third_party/spdlog)
    set(FMT_TARGET fmt_local)
    message("Using built-in version of fmt")
endif()

# Define two variables in order not to repeat ourselves.
if(${CMAKE_VERSION} VERSION_LESS "3.13.0") 
    link_directories(${SystemC_LIBRARY_DIRS})
endif()

# Define the scc library
add_library(scc INTERFACE)
target_include_directories (scc INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories (scc INTERFACE ${Boost_INCLUDE_DIRS})
if(HAVE_GETENV)
    target_compile_definitions(scc INTERFACE HAVE_GETENV)
endif()

target_link_libraries(scc INTERFACE scc-util scc-sysc components busses)
target_link_libraries(scc INTERFACE jsoncpp)
target_link_libraries(scc INTERFACE ${Boost_LIBRARIES})
target_link_libraries(scc INTERFACE ${FMT_TARGET} ${SPDLOG_TARGET})

set_target_properties(scc PROPERTIES
	  PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/src/scc.h
)

install(TARGETS scc 
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )

###############################################################################
# subdirectories
###############################################################################
add_subdirectory(src/common)
add_subdirectory(src/bus_interfaces)
add_subdirectory(src/components)
add_subdirectory(src/sysc)
add_subdirectory(third_party)
if(NOT SCC_LIB_ONLY)
	add_subdirectory(examples)
endif()

