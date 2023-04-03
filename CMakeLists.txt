cmake_minimum_required(VERSION 3.16)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_CURRENT_BINARY_DIR}) # project specific cmake dir
cmake_policy(SET CMP0077 NEW)

project(scc VERSION 2022.4.0 LANGUAGES CXX C)

option(USE_CWR_SYSTEMC "Use Synopsys Virtualizer SystemC" OFF)

option(USE_NCSC_SYSTEMC "Cadence Xcelium SystemC" OFF)

option(ENABLE_CONAN "Enable the use of conan in standalone build" ON)

#Note: this needs to match the SystemC kernel build options
option(SC_WITH_PHASE_CALLBACKS "Whether SystemC is built with simulation phase callbacks" OFF)

option(SC_WITH_PHASE_CALLBACK_TRACING "whether SystemC was build with pahse callbacks for tracing. It needs to match the SystemC build configuration" OFF)

set(SCC_ARCHIVE_DIR_MODIFIER "" CACHE STRING "additional directory levels to store static library archives") 

set(SCC_LIBRARY_DIR_MODIFIER "" CACHE STRING "additional directory levels to store static library archives") 

include(Common)

if(CMAKE_PROJECT_NAME STREQUAL "scc")
    message(STATUS "Building SCC in standalone mode")
    include(GNUInstallDirs)
    if(ENABLE_CONAN)
        include(ConanInline)
        conan_check()
        if(EXISTS  /etc/redhat-release)
	        # Boost on CentOS quirks: the b2 of conan-center is build against a newer libstdc++ and therefore does not run
			# with the oooooold libs on CentOS 7. Therefore we build our own version of b2 if it is not there
			set(B2_VERSION 4.9.2)
			set(B2_META $ENV{HOME}/.conan/data/b2/${B2_VERSION}/_/_/metadata.json)
			if(DEFINED ENV{CONAN_USER_HOME})
				set(B2_META $ENV{CONAN_USER_HOME}/.conan/data/b2/${B2_VERSION}/_/_/metadata.json)
			endif()
			if(NOT EXISTS ${B2_META})
				conan_configure(REQUIRES b2/${B2_VERSION})
				conan_cmake_autodetect(settings)
				conan_cmake_install(PATH_OR_REFERENCE . BUILD b2 SETTINGS ${settings})
			endif()
			# Boost on CentOS quirks end
        endif()
        set(CONAN_PACKAGE_LIST jsoncpp/1.9.5 yaml-cpp/0.6.3 spdlog/1.9.2 fmt/8.0.1 zlib/1.2.12 boost/1.75.0)
		if(BUILD_SCC_DOCUMENTATION)
        	list(APPEND CONAN_PACKAGE_LIST doxygen/1.9.2)
        endif()
        set(CONAN_PACKAGE_OPTIONS fmt:header_only=True spdlog:header_only=True boost:without_stacktrace=True boost:shared=False boost:header_only=False)
        if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
            list(APPEND CONAN_PACKAGE_OPTIONS boost:fPIC=True)
        endif()
        if(NOT USE_CWR_SYSTEMC AND NOT USE_NCSC_SYSTEMC AND NOT DEFINED ENV{SYSTEMC_HOME})
            set(CONAN_PACKAGE_LIST ${CONAN_PACKAGE_LIST} systemc/2.3.3 systemc-cci/1.0.0)
            set(CONAN_PACKAGE_OPTIONS ${CONAN_PACKAGE_OPTIONS} systemc-cci:shared=False)
        endif()
        conan_configure(REQUIRES ${CONAN_PACKAGE_LIST} GENERATORS cmake_find_package OPTIONS ${CONAN_PACKAGE_OPTIONS})
        if(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
            conan_install(BUILD_TYPE Release)
        else()
            conan_install()
        endif()
        set(CONAN_CMAKE_SILENT_OUTPUT ON)
        find_package(spdlog REQUIRED)
        find_package(lz4 REQUIRED)
        find_package(fmt REQUIRED)
        find_package(yaml-cpp REQUIRED)
    endif()
	
    set(Boost_NO_BOOST_CMAKE ON) #  Don't do a find_package in config mode before searching for a regular boost install.
    option(ENABLE_CLANG_TIDY "Add clang-tidy automatically to builds" OFF)
    option(BUILD_SCC_LIB_ONLY "Build only the library (no examples" OFF)
    option(BUILD_SCC_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ON)
    option(INSTALL_DEPENDENCIES "Should dependencies be installed when installing SCC" OFF)
else()
    option(BUILD_SCC_LIB_ONLY "Build only the library (no examples" ON)
    option(BUILD_SCC_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" OFF)
endif()

if(BUILD_SCC_DOCUMENTATION)
	add_subdirectory(doc)
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
include(SystemCPackage)

include(CheckSymbolExists)
# Check for function getenv()
check_symbol_exists(getenv "stdlib.h" HAVE_GETENV)

# check which version of spdlog to use.
if(TARGET spdlog::spdlog)
    set(SPDLOG_TARGET spdlog::spdlog)
else()
    add_library(spdlog_local INTERFACE IMPORTED)
   	if(TARGET fmt::fmt)
    	set_property(TARGET spdlog_local PROPERTY INTERFACE_COMPILE_DEFINITIONS SPDLOG_HEADER_ONLY SPDLOG_FMT_EXTERNAL)
    else()
    	set_property(TARGET spdlog_local PROPERTY INTERFACE_COMPILE_DEFINITIONS SPDLOG_HEADER_ONLY)
    endif()
    set_property(TARGET spdlog_local PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/third_party/spdlog-1.8)
    set(SPDLOG_TARGET spdlog_local)
    message(STATUS "${PROJECT_NAME}: using built-in version of spdlog")
endif()

# check which version of fmt to use
if(TARGET fmt::fmt)
    set(FMT_TARGET fmt::fmt)
else()
    add_library(fmt_local INTERFACE IMPORTED)
    set_property(TARGET fmt_local PROPERTY INTERFACE_COMPILE_DEFINITIONS FMT_SPDLOG_INTERNAL)
    set_property(TARGET fmt_local PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/third_party/spdlog-1.8/spdlog)
    set(FMT_TARGET fmt_local)
    message(STATUS "${PROJECT_NAME}: using built-in version of fmt")
endif()

if(NOT TARGET lz4::lz4)
    message(STATUS "${PROJECT_NAME}: using built-in version of lz4")
	add_subdirectory(third_party/lz4-1.9.4)
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
target_link_libraries(scc INTERFACE ${FMT_TARGET} ${SPDLOG_TARGET})

set_target_properties(scc PROPERTIES
    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/src/scc.h
)

install(TARGETS scc
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_LIBRARY_DIR_MODIFIER}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_LIBRARY_DIR_MODIFIER}${SCC_ARCHIVE_DIR_MODIFIER}
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
###############################################################################
# install dependend libs
###############################################################################
install(DIRECTORY contrib/pysysc contrib/d3-hwschematic
		DESTINATION share
		PATTERN ".gitignore" EXCLUDE 
		PATTERN "PySysC_SCC.egg*" EXCLUDE
		PATTERN "build" EXCLUDE
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

