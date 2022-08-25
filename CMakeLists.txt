cmake_minimum_required(VERSION 3.16)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_CURRENT_BINARY_DIR}) # project specific cmake dir
cmake_policy(SET CMP0077 NEW)

project(scc VERSION 2022.4.0 LANGUAGES CXX C)

option(USE_CWR_SYSTEMC "Use Synopsys Virtualizer SystemC" OFF)

option(USE_NCSC_SYSTEMC "Cadence Xcelium SystemC" OFF)

#Note: this needs to match the SystemC kernel build options
option(SC_WITH_PHASE_CALLBACKS "Whether SystemC is built with simulation phase callbacks" OFF)

option(SC_WITH_PHASE_CALLBACK_TRACING "whether SystemC was build with pahse callbacks for tracing. It needs to match the SystemC build configuration" OFF)

set(SCC_ARCHIVE_DIR_MODIFIER "" CACHE STRING "additional directory levels to store static library archives") 

include(Common)

if(CMAKE_PROJECT_NAME STREQUAL "scc")
    message(STATUS "Building SCC in standalone mode")
    include(GNUInstallDirs)
    include(ConanInline)
    conan_check()
    set(CONAN_PACKAGE_LIST fmt/8.0.1 zlib/1.2.11 boost/1.75.0 gsl-lite/0.37.0)
    set(CONAN_PACKAGE_OPTIONS fmt:header_only=True boost:without_stacktrace=True boost:shared=False boost:header_only=False)
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        list(APPEND CONAN_PACKAGE_OPTIONS boost:fPIC=True)
    endif()
    if(NOT USE_CWR_SYSTEMC AND NOT USE_NCSC_SYSTEMC AND NOT DEFINED ENV{SYSTEMC_HOME})
        set(CONAN_PACKAGE_LIST ${CONAN_PACKAGE_LIST} systemc/2.3.3 systemc-cci/1.0.0)
        set(CONAN_PACKAGE_OPTIONS ${CONAN_PACKAGE_OPTIONS} systemc-cci:shared=False)
    endif()
    conan_configure(REQUIRES ${CONAN_PACKAGE_LIST} GENERATORS cmake cmake_find_package OPTIONS ${CONAN_PACKAGE_OPTIONS})
    if(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        conan_install(BUILD_TYPE Release)
    else()
        conan_install()
    endif()
    conan_setup(TARGETS)
    set(Boost_NO_BOOST_CMAKE ON) #  Don't do a find_package in config mode before searching for a regular boost install.
    option(ENABLE_CLANG_TIDY "Add clang-tidy automatically to builds" OFF)
    option(BUILD_SCC_LIB_ONLY "Build only the library (no examples" OFF)
    option(BUILD_SCC_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ON)
else()
    option(BUILD_SCC_LIB_ONLY "Build only the library (no examples" ON)
    option(BUILD_SCC_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" OFF)
endif()

if(BUILD_SCC_DOCUMENTATION)
find_package(Doxygen)
    if(NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "Doxygen is needed to build the documentation.")
    endif()
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in)
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    
    set(DOXYGEN_PROJECT_BRIEF "SystemC components library")
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE  README.md)
    # exclude json code
    # doxygen settings can be set here, prefixed with "DOXYGEN_"
    set(DOXYGEN_SOURCE_BROWSER YES)
    set(DOXYGEN_EXTRACT_ALL NO)
    set(DOXYGEN_EXTRACT_PRIVATE NO)
    set(DOXYGEN_SEARCHENGINE NO)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    set(DOXYGEN_GENERATE_LATEX NO)
    set(DOXYGEN_OUTPUT_DIRECTORY "${scc_BINARY_DIR}/scc")
    set(DOXYGEN_GENERATE_TREEVIEW  YES)
    set(DOXYGEN_HTML_EXTRA_STYLESHEET ${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen-awesome.css)
    #set(DOXYGEN_HTML_EXTRA_STYLESHEET ${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen-awesome.css ${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen-awesome-sidebar-only.css)
    # this target will only be built if specifically asked to.
    # run "make scc-docs" to create the doxygen documentation
    doxygen_add_docs(
        scc-docs
        ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/third_party/axi_chi/axi  ${PROJECT_SOURCE_DIR}/third_party/axi_chi/chi  ${PROJECT_SOURCE_DIR}/README.md
        ALL COMMENT "Generate API-documents for SCC."
    )
    install(DIRECTORY ${scc_BINARY_DIR}/scc DESTINATION ${CMAKE_INSTALL_DOCDIR}/)
endif()


###############################################################################
# build the SCC
###############################################################################
if (ENABLE_CLANG_TIDY)
    set(CLANG_FORMAT_EXCLUDE_PATTERNS "third_party/fst" "fmt_8.0" "jsoncpp-1.8" "rapidjson-1.1" "spdlog-1.8" "sqlite3")
    find_package(ClangFormat)
    find_program (CLANG_TIDY_EXE NAMES "clang-tidy" PATHS /usr/bin )
    if (CLANG_TIDY_EXE)
        message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
        set(CLANG_TIDY_CHECKS "-*")
        set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},modernize-avoid-bind.PermissiveParameterList")
        set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},modernize-loop-convert.*")
        set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},modernize-make-shared.")
        set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},modernize-make-unique.")
        set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},modernize-pass-by-value.*")
        set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},modernize-raw-string-literal.*")
        set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},modernize-replace-auto-ptr.IncludeStyle")
        set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},modernize-replace-disallow-copy-and-assign-macro.MacroName")
        set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},modernize-replace-random-shuffle.IncludeStyle")
        set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},modernize-use-auto.*")
        set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},modernize-use-bool-literals.IgnoreMacros")
        set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},modernize-use-default-member-init.*")
        set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},modernize-use-emplace.*")
        set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},modernize-use-equals-default.IgnoreMacros")
        set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},modernize-use-equals-delete.IgnoreMacros")
        set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},modernize-use-nodiscard.ReplacementString")
        set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},modernize-use-noexcept.*")
        set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},modernize-use-nullptr.NullMacros")
        set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},modernize-use-override.*")
        set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},modernize-use-transparent-functors.SafeMode")
        set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},modernize-use-using.IgnoreMacros")
        set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},cppcoreguidelines-explicit-virtual-functions.IgnoreDestructors")
        #set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},cppcoreguidelines-*")
        set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},clang-diagnostic-*,clang-analyzer-*")
        set(DO_CLANG_TIDY "${CLANG_TIDY_EXE};-checks=${CLANG_TIDY_CHECKS};-header-filter='${CMAKE_SOURCE_DIR}/*';-fix"
            CACHE STRING "" FORCE)
    else()
        message(AUTHOR_WARNING "clang-tidy not found!")
        set(CMAKE_CXX_CLANG_TIDY "" CACHE STRING "" FORCE) # delete it
    endif()
endif()

find_package(Boost REQUIRED COMPONENTS system date_time) # header only libraries must not be added here

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
    set_property(TARGET spdlog_local PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/third_party/spdlog-1.8)
    set(SPDLOG_TARGET spdlog_local)
    message(STATUS "${PROJECT_NAME}: using built-in version of spdlog")
endif()

# check which version of fmt to use
if(TARGET fmt::fmt)
    set(FMT_TARGET fmt::fmt)
elseif(TARGET CONAN_PKG::fmt)
    set(FMT_TARGET CONAN_PKG::fmt)
else()
    add_library(fmt_local INTERFACE IMPORTED)
    set_property(TARGET fmt_local PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/third_party/spdlog-1.8/spdlog)
    set(FMT_TARGET fmt_local)
    message(STATUS "${PROJECT_NAME}: using built-in version of fmt")
endif()

# Define the scc library
add_library(scc INTERFACE)
if(HAVE_GETENV)
    target_compile_definitions(scc INTERFACE HAVE_GETENV)
endif()

target_include_directories (scc INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
if(TARGET Boost::date_time)
    target_link_libraries(scc INTERFACE Boost::date_time)
else()
    target_include_directories(scc INTERFACE ${Boost_INCLUDE_DIRS})
    target_link_libraries(scc INTERFACE ${Boost_datetime_LIBRARY})
endif()
target_link_libraries(scc INTERFACE scc-util scc-sysc components busses scv-tr)
#target_link_libraries(scc INTERFACE jsoncpp)
target_link_libraries(scc INTERFACE ${FMT_TARGET} ${SPDLOG_TARGET})

set_target_properties(scc PROPERTIES
      PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/src/scc.h
)

install(TARGETS scc
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_ARCHIVE_DIR_MODIFIER}
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
    if (NOT (DEFINED CMAKE_CXX_CLANG_TIDY OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
        add_subdirectory(examples)
    endif()
endif()
