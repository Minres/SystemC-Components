cmake_minimum_required(VERSION 3.20)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_CURRENT_BINARY_DIR}) # project specific cmake dir
cmake_policy(SET CMP0077 NEW)

project(scc VERSION 2025.09 LANGUAGES CXX C)

option(USE_CWR_SYSTEMC "Use Synopsys Virtualizer SystemC" OFF)
option(USE_NCSC_SYSTEMC "Cadence Xcelium SystemC" OFF)
option(BUILD_TESTING "Enable building tests" OFF)
option(BUILD_SCC_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" OFF)
option(FULL_TRACE_TYPE_LIST "Test for extended set of templated datatypes" OFF)
#Note: this needs to match the SystemC kernel build options
option(SC_WITH_PHASE_CALLBACKS "Whether SystemC is built with simulation phase callbacks" OFF)
option(SC_WITH_PHASE_CALLBACK_TRACING "whether SystemC was build with phase callbacks for tracing. It needs to match the SystemC build configuration" OFF)
set(SCC_ARCHIVE_DIR_MODIFIER "" CACHE STRING "additional directory levels to store static library archives") 
set(SCC_LIBRARY_DIR_MODIFIER "" CACHE STRING "additional directory levels to store libraries") 
option(WITH_SCP4SCC "adds SCP compatibility layer for SCC, cannot be used together with a copy of SCP" OFF)
option(ENABLE_CLANG_FORMAT "Enable clang-format targets" OFF)

include(Common)

if(CMAKE_PROJECT_NAME STREQUAL "scc")
    message(STATUS "Building SCC in standalone mode")
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
	set(CMAKE_CXX_EXTENSIONS OFF)
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)

    include(GNUInstallDirs)
    
    set(Boost_NO_BOOST_CMAKE ON) #  Don't do a find_package in config mode before searching for a regular boost install.
    option(ENABLE_CLANG_TIDY "Add clang-tidy automatically to builds" OFF)
    option(BUILD_SCC_LIB_ONLY "Build only the library (no examples" OFF)
    option(INSTALL_DEPENDENCIES "Should dependencies be installed when installing SCC" OFF)
	set(CLANG_FORMAT_EXCLUDE_PATTERNS "/third_party/" "/build/" ".direnv")
    if(ENABLE_CLANG_FORMAT)
	    find_package(ClangFormat)
    endif()
else()
    option(BUILD_SCC_LIB_ONLY "Build only the library (no examples" ON)
	set(CLANG_FORMAT_EXCLUDE_PATTERNS ${CLANG_FORMAT_EXCLUDE_PATTERNS} "/third_party/" PARENT_SCOPE)
endif()

if(BUILD_SCC_DOCUMENTATION)
    add_subdirectory(doc)
endif()
set(SCC_CMAKE_CONFIG_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/scc)


###############################################################################
# build the SCC
###############################################################################
if (ENABLE_CLANG_TIDY)
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

find_package(Boost REQUIRED QUIET COMPONENTS date_time) # header only libraries must not be added here
find_package(lz4 QUIET)
if(NOT ${lz4_FOUND})
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(LIBLZ4 QUIET IMPORTED_TARGET liblz4)
        if(LIBLZ4_FOUND)
            add_library(lz4::lz4 ALIAS PkgConfig::LIBLZ4)
        endif()
    endif()
endif()
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(yaml-cpp REQUIRED)
if(MSVC)
    add_compile_options(/vmg /MP /W3 /wd4244 /wd4267 /wd4996  -DNOMINMAX /EHsc)
    link_directories(${Boost_LIBRARY_DIRS})
endif()
include(SystemCPackage)
	
include(CheckSymbolExists)
# Check for function getenv()
check_symbol_exists(getenv "stdlib.h" HAVE_GETENV)

if(BUILD_TESTING)
    include(CTest)
endif()

if(NOT TARGET lz4::lz4)
    message(STATUS "${PROJECT_NAME}: using built-in version of lz4")
    add_subdirectory(third_party/lz4-1.9.4)
endif()

###############################################################################
# subdirectories
###############################################################################
add_subdirectory(src/common)
if(SystemC_FOUND)
    add_subdirectory(src/interfaces)
    add_subdirectory(src/components)
    add_subdirectory(src/sysc)
    add_subdirectory(third_party)
    if(NOT BUILD_SCC_LIB_ONLY)
        if (NOT (DEFINED CMAKE_CXX_CLANG_TIDY OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
            add_subdirectory(examples)
        endif()
    endif()

    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif()

    # Define the scc library
    add_library(scc INTERFACE)
    add_library(scc::scc ALIAS scc)
    if(HAVE_GETENV)
        target_compile_definitions(scc INTERFACE HAVE_GETENV)
    endif()
    
    target_include_directories(scc INTERFACE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
    if(TARGET Boost::date_time)
        target_link_libraries(scc INTERFACE Boost::date_time)
    else()
        target_include_directories(scc INTERFACE ${Boost_INCLUDE_DIRS})
        target_link_libraries(scc INTERFACE ${Boost_datetime_LIBRARY})
    endif()
    target_link_libraries(scc INTERFACE scc-util scc-sysc components interfaces scv-tr)
    set_target_properties(scc PROPERTIES
        PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/src/scc.h
    )

	if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
		#set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--export-all-symbols")
		add_library(scc_shared SHARED $<TARGET_OBJECTS:scc-util>
	                                  $<TARGET_OBJECTS:scc-sysc>
	                                  $<TARGET_OBJECTS:interfaces>
	                                  $<TARGET_OBJECTS:scv-tr>
	                                  )
	    set_target_properties(scc_shared PROPERTIES CXX_VISIBILITY_PRESET hidden)
		if(TARGET Boost::stacktrace_backtrace)
		    target_link_libraries(scc_shared PUBLIC Boost::stacktrace_backtrace dl)
		endif()
	    add_library(scc::scc_shared ALIAS scc_shared)
	endif()
    install(TARGETS scc
        EXPORT scc-targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_LIBRARY_DIR_MODIFIER}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_LIBRARY_DIR_MODIFIER}${SCC_ARCHIVE_DIR_MODIFIER}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
    install(EXPORT scc-targets 
        FILE scc-targets.cmake
        DESTINATION ${SCC_CMAKE_CONFIG_DIR}
        NAMESPACE scc::
        )
	if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	    install(TARGETS scc_shared
	        EXPORT scc_shared-targets
	        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_LIBRARY_DIR_MODIFIER}
	        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_LIBRARY_DIR_MODIFIER}${SCC_ARCHIVE_DIR_MODIFIER}
	        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	        )
	    install(EXPORT scc_shared-targets 
	        FILE scc_shared-targets.cmake
	        DESTINATION ${SCC_CMAKE_CONFIG_DIR}
	        NAMESPACE scc::
	        )
	endif()
    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/scc-config-version.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
    )
    
    configure_package_config_file(
        ${CMAKE_CURRENT_LIST_DIR}/cmake/scc-config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/scc-config.cmake
        INSTALL_DESTINATION ${SCC_CMAKE_CONFIG_DIR}
    )

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/scc-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/scc-config-version.cmake
        DESTINATION ${SCC_CMAKE_CONFIG_DIR})
else()
    add_subdirectory(third_party)
endif()
###############################################################################
# install dependend libs
###############################################################################
install(DIRECTORY contrib/pysysc contrib/d3-hwschematic
    DESTINATION share
    PATTERN ".gitignore" EXCLUDE 
    PATTERN "PySysC_SCC.egg*" EXCLUDE
    PATTERN "build" EXCLUDE
    )
install(DIRECTORY cmake
    DESTINATION share
    )
if(CMAKE_PROJECT_NAME STREQUAL "scc")
    if(INSTALL_DEPENDENCIES)
        install(CODE "set(CMAKE_INSTALL_LIBDIR \"${CMAKE_INSTALL_LIBDIR}\")")
        install(CODE [[    
          file(GET_RUNTIME_DEPENDENCIES
            LIBRARIES $<TARGET_FILE:scc-sysc>
            RESOLVED_DEPENDENCIES_VAR _r_deps
            UNRESOLVED_DEPENDENCIES_VAR _u_deps
            CONFLICTING_DEPENDENCIES_PREFIX _c_deps
          )
          foreach(_file ${_c_deps_FILENAMES})
                set(FLIST ${_c_deps_${_file}})
                list(LENGTH FLIST LIST_LEN)
                list(GET FLIST -1 FNAME)
                message(STATUS "Conflicting files for ${_file} are ${_c_deps_${_file}}, using ${FNAME}")
                list(APPEND _r_deps ${FNAME})
          endforeach()
          foreach(_file ${_r_deps})
            if(${_file} MATCHES "libz" OR NOT (${_file} MATCHES "^/lib")) # don't copy system libraries except libz
                file(INSTALL
                  DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}
                  TYPE SHARED_LIBRARY
                  FOLLOW_SYMLINK_CHAIN
                  PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
                  FILES "${_file}"
                )
            endif()
          endforeach()
          list(LENGTH _u_deps _u_length)
          if("${_u_length}" GREATER 0)
            message(WARNING "Unresolved dependencies detected: '${_u_deps}'!")
          endif()
        ]])
    endif()
endif()
