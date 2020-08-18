cmake_minimum_required(VERSION 3.11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake) # project specific cmake dir

project(scc VERSION 0.9.0 LANGUAGES CXX C)

include(Common)
include(GNUInstallDirs)

###############################################################################
# build a separate SCC-UTIL library for the use in non-systemc based systems
###############################################################################
add_library(scc-util STATIC src/io-redirector.cpp)
target_include_directories (scc-util PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/incl)

FILE(GLOB SccUtilHeaders incl/util/*.h)
set_target_properties(scc-util PROPERTIES
    VERSION ${PROJECT_VERSION}  # ${VERSION} was defined in the main CMakeLists.
    FRAMEWORK FALSE
    PUBLIC_HEADER "${SccUtilHeaders}" # specify the public headers
)

install(TARGETS scc-util
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/util
)

###############################################################################
# build a SCC
###############################################################################
if(NOT SystemCPackage)
    if(TARGET SystemC::SystemC)
        set(SystemCPackage SystemC)
    else()
        set(SystemCPackage OSCISystemC)
    endif()
endif()

message("Using ${SystemCPackage} package")

if(TARGET SystemCVerification::SystemCVerification)
    set(SCV_FOUND TRUE)
    set(SCV_LIBRARIES SystemCVerification::SystemCVerification)
endif()
if(TARGET SystemC-CCI::SystemC-CCI)
    set(CCI_FOUND TRUE)
    set(CCI_LIBRARIES SystemC-CCI::SystemC-CCI)
endif()

find_package(Threads)
find_package(Boost REQUIRED COMPONENTS date_time)
find_package(ZLIB REQUIRED)
find_package(${SystemCPackage} REQUIRED)

set(CLANG_FORMAT_EXCLUDE_PATTERNS "/spdlog/" "/json/" "jsoncpp.cpp" "sqlite3")
find_package(ClangFormat)

include(CheckSymbolExists)
# Check for function getenv()
check_symbol_exists(getenv "stdlib.h" HAVE_GETENV)

set(LIB_SOURCES
    src/sc_logic_7.cpp
    src/utilities.cpp
    src/report.cpp
    src/perf_estimator.cpp
    src/io-redirector.cpp
    src/ordered_semaphore.cpp
    src/configurer.cpp
    src/jsoncpp.cpp
    src/tlm_ahb_bfm_initiator.cpp
    src/tlm_ahb_bfm_target.cpp
    src/ahb_initiator.cpp
 	src/ahb_target.cpp
 	src/apb_target.cpp
    src/tracer_base.cpp
    src/tracer.cpp
    src/mt19937_rng.cpp
)

if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.9)
set(LIB_SOURCES ${LIB_SOURCES} src/watchdog.cpp)
endif()

if(CCI_FOUND)
    set(LIB_SOURCES ${LIB_SOURCES} 
        src/configurable_tracer.cpp
        src/value_registry.cpp
    )
endif()

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

if(SCV_FOUND )
    if(ZLIB_FOUND)
        set(LIB_SOURCES ${LIB_SOURCES} src/scv_tr_compressed.cpp)
    endif(ZLIB_FOUND)
    if(FALSE)
        set(LIB_SOURCES ${LIB_SOURCES} src/scv_tr_binary/scv_tr_binary.cpp)
        if(CONAN_LEVELDB_ROOT)
            set(LIB_SOURCES ${LIB_SOURCES} src/scv_tr_ldb/scv_tr_ldb.cpp )
        endif()
    endif()
    set(LIB_SOURCES ${LIB_SOURCES} src/scv_tr_sqlite/scv_tr_sqlite.cpp src/scv_tr_sqlite/sqlite3.c )
endif()

# Define two variables in order not to repeat ourselves.
if(${CMAKE_VERSION} VERSION_LESS "3.13.0") 
    link_directories(${SystemC_LIBRARY_DIRS})
endif()

# Define the scc library
add_library(scc ${LIB_SOURCES})
target_include_directories (scc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/incl)
target_include_directories (scc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories (scc PUBLIC ${Boost_INCLUDE_DIRS})
if(HAVE_GETENV)
    target_compile_definitions(scc PUBLIC HAVE_GETENV)
endif()
if (ZLIB_FOUND)
    target_include_directories(scc PUBLIC ${ZLIB_INCLUDE_DIRS})
    target_link_libraries(scc PUBLIC ${ZLIB_LIBRARIES})
endif()
target_link_libraries(scc PUBLIC scc-util ${CMAKE_DL_LIBS})
target_link_libraries(scc PUBLIC ${Boost_LIBRARIES})
target_link_libraries(scc PUBLIC ${FMT_TARGET} ${SPDLOG_TARGET})
target_link_libraries(scc PUBLIC Threads::Threads)

if(CCI_FOUND)
	message("Building SCC with CCI support")
    target_compile_definitions(scc PUBLIC -DWITH_CCI)
    target_include_directories (scc PUBLIC ${CCI_INCLUDE_DIRS})
    target_link_libraries(scc PUBLIC ${CCI_LIBRARIES} )
    if(NOT ${CMAKE_VERSION} VERSION_LESS "3.13.0") 
        target_link_directories(scc PUBLIC ${CCI_LIBRARY_DIRS})
    endif()
endif()

if(SCV_FOUND)   
	message("Building SCC with SCV support")
    target_compile_definitions(scc PUBLIC -DWITH_SCV)
    target_include_directories (scc PUBLIC ${SCV_INCLUDE_DIRS})
    target_link_libraries (scc PUBLIC ${SCV_LIBRARIES})
    if(NOT ${CMAKE_VERSION} VERSION_LESS "3.13.0") 
        target_link_directories(scc PUBLIC ${SCV_LIBRARY_DIRS})
    endif()
endif()

if(ZLIB_FOUND)
    target_link_libraries (scc PUBLIC ${ZLIB_LIBRARIES})
endif(ZLIB_FOUND)

target_include_directories (scc PUBLIC ${SystemC_INCLUDE_DIRS})    
target_link_libraries(scc PUBLIC ${SystemC_LIBRARIES} )
if(NOT ${CMAKE_VERSION} VERSION_LESS "3.13.0") 
    target_link_directories(scc PUBLIC ${SystemC_LIBRARY_DIRS})
endif()

set_target_properties(scc PROPERTIES
	VERSION ${PROJECT_VERSION}
  FRAMEWORK FALSE
  PUBLIC_HEADER "incl/scc.h" # specify the public headers
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
# build a separate JSON library for the use in non-systemc based systems
###############################################################################
add_library(jsoncpp STATIC src/jsoncpp.cpp)
target_include_directories (jsoncpp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/incl)

FILE(GLOB JsonLibHeaders incl/json/*.h)
set_target_properties(jsoncpp PROPERTIES
    VERSION 1.8.3
    FRAMEWORK FALSE
    PUBLIC_HEADER "${JsonLibHeaders}" # specify the public headers
)

install(TARGETS jsoncpp
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/json
        )
