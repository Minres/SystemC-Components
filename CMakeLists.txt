cmake_minimum_required(VERSION 3.11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake) # project specific cmake dir

project(scc VERSION 1.0.0 LANGUAGES CXX C)

option(BUILD_SCC_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ${DOXYGEN_FOUND})

option(BUILD_STAND_ALONE "Do a standalone build and do not rely on a parent project providing needed libraries" FALSE)
include(Common)
include(GNUInstallDirs)
if(BUILD_STAND_ALONE)
	include(Conan)
	setup_conan(TARGETS)
endif()


find_package(Doxygen)

if(BUILD_SCC_DOCUMENTATION)
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
	set(DOXYGEN_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/docs")
	# this target will only be built if specifically asked to.
	# run "make scc-docs" to create the doxygen documentation
	doxygen_add_docs(
	    scc-docs
	    ${PROJECT_SOURCE_DIR}/incl ${PROJECT_SOURCE_DIR}/README.md
	    ALL COMMENT "Generate API-documents for SCC."
	)
    install(DIRECTORY ${PROJECT_BINARY_DIR}/docs TYPE DOC)
endif()


###############################################################################
# build the SCC
###############################################################################

set(CLANG_FORMAT_EXCLUDE_PATTERNS "/third_party/" "sqlite3")
find_package(ClangFormat)
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

target_link_libraries(scc INTERFACE scc-util sysc components busses)
target_link_libraries(scc INTERFACE jsoncpp)
target_link_libraries(scc INTERFACE ${Boost_LIBRARIES})
target_link_libraries(scc INTERFACE ${FMT_TARGET} ${SPDLOG_TARGET})

set_target_properties(scc PROPERTIES
	VERSION ${PROJECT_VERSION}
  FRAMEWORK FALSE
  PUBLIC_HEADER "scc.h" # specify the public headers
)
install(TARGETS scc
        RUNTIME DESTINATION ${CMAKE_INSTALL_LCMAKE_INSTALL_BINDIRIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
install(
    DIRECTORY   ${CMAKE_CURRENT_SOURCE_DIR}/incl/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*")
###############################################################################
# subdirectories
###############################################################################
add_subdirectory(examples)
add_subdirectory(src/common)
add_subdirectory(src/bus_interfaces)
add_subdirectory(src/components)
add_subdirectory(src/sysc)
add_subdirectory(third_party)

