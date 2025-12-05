project(scc-sysc VERSION 0.0.1 LANGUAGES CXX)

find_package(Threads)
find_package(Boost REQUIRED QUIET COMPONENTS date_time filesystem)
find_package(Boost QUIET COMPONENTS stacktrace_backtrace)
find_package(ZLIB)

option(ENABLE_SQLITE "Enable SQLite backend for SCV" ON)
option(ENABLE_PYTHON4SC "Enable Python interpreter integration" OFF)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif()

if(SystemC_FOUND)
    message(STATUS "SystemC headers at ${SystemC_INCLUDE_DIRS}")
    message(STATUS "SystemC libraries: ${SystemC_LIBRARIES}")
    if(SCV_FOUND)
        message(STATUS "SCV headers at ${SCV_INCLUDE_DIRS}")
        message(STATUS "SCV libraries: ${SCV_LIBRARIES}")
    endif(SCV_FOUND)
else()
    message(FATAL_ERROR "No usable SystemC installation found")
endif()

set(LIB_SOURCES
    scc/utilities.cpp 
    scc/tracer_base.cpp
    scc/tracer.cpp
    scc/perf_estimator.cpp
    scc/sc_logic_7.cpp
    scc/report.cpp
    scc/ordered_semaphore.cpp
    scc/mt19937_rng.cpp
    scc/time_n_tick.cpp
    #scc/scv/scv_tr_binary.cpp
    scc/scv/scv_tr_mtc.cpp
    scc/scv/scv_tr_lz4.cpp
    scc/scv/scv_tr_ftr.cpp
    scc/vcd_pull_trace.cpp
    scc/vcd_push_trace.cpp
    scc/hierarchy_dumper.cpp
    scc/cci_broker.cpp
    scc/configurer.cpp
    scc/configurable_tracer.cpp
    scc/sc_thread_pool.cpp
    scc/signal_opt_ports.cpp
    tlm/scc/scv/tlm_recorder.cpp
    tlm/scc/pe/parallel_pe.cpp
    tlm/scc/lwtr/tlm2_lwtr.cpp
)

if(DEFINED  SC_VERSION_MAJOR AND SC_VERSION_MAJOR GREATER 2)
    list(APPEND LIB_SOURCES tlm/scc/quantum_keeper.cpp)
endif()
if(ENABLE_PYTHON4SC)
    find_package(pybind11 QUIET)
    if(pybind11_NOT_FOUND)
        set(PYBIND11_INSTALL ON)
        include(FetchContent)
        FetchContent_Declare(
            pybind11
            GIT_REPOSITORY https://github.com/pybind/pybind11.git
            GIT_TAG        stable
        )
        FetchContent_MakeAvailable(pybind11)
    endif()
    list(APPEND LIB_SOURCES  
        scc/python4sc.cpp
        )
endif()

if(ZLIB_FOUND)
    message(STATUS "${PROJECT_NAME}: building zlib based parts")
    list(APPEND LIB_SOURCES  
        scc/scv/scv_tr_compressed.cpp
        scc/fst_trace.cpp
        # scc/vcd_mt_trace.cpp
        )
    set(WITH_FST ON)
endif()

if(ENABLE_SQLITE)
    list(APPEND LIB_SOURCES  scc/scv/scv_tr_sqlite.cpp ../../third_party/sqlite3/sqlite3.c )
endif()

add_library(${PROJECT_NAME} ${LIB_SOURCES})
add_library(scc::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
if(FULL_TRACE_TYPE_LIST)
    target_compile_definitions(${PROJECT_NAME} PRIVATE FULL_TRACE_TYPE_LIST)
endif()
if(SC_WITH_PHASE_CALLBACKS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE WITH_SC_PHASE_CALLBACKS)
endif()
if(SC_WITH_PHASE_CALLBACK_TRACING)
    target_compile_definitions(${PROJECT_NAME} PUBLIC WITH_SC_TRACING_PHASE_CALLBACKS)
endif()
target_include_directories (${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> # for headers when building
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> # for client in install mode
)
target_link_libraries(${PROJECT_NAME} PUBLIC scc-util RapidJSON::RapidJSON lwtr)
if(ENABLE_PYTHON4SC)
    target_compile_options(${PROJECT_NAME} PRIVATE -fvisibility=hidden)
    target_link_libraries(${PROJECT_NAME} PUBLIC pybind11::embed) #pybind11::headers)
endif()
if(TARGET fmt::fmt-header-only)
	target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt-header-only)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)
endif()
if(TARGET spdlog::spdlog_header_only)
	target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog_header_only)
else()
	target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)
endif()
if(ENABLE_SQLITE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE WITH_SQLITE)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/sqlite3)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC Boost::date_time)
if(TARGET Boost::stacktrace_backtrace)
    message(STATUS "scc-sysc: using stacktrace_backtrace for reporting")
    target_compile_definitions(${PROJECT_NAME} PRIVATE WITH_STACKTRACE BOOST_STACKTRACE_LINK BOOST_STACKTRACE_USE_BACKTRACE)
    target_link_libraries(${PROJECT_NAME} PUBLIC Boost::stacktrace_backtrace dl)
endif()
find_package(yaml-cpp QUIET)
if(TARGET yaml-cpp::yaml-cpp)
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DHAS_YAMPCPP)
    target_link_libraries(${PROJECT_NAME} PUBLIC yaml-cpp::yaml-cpp)
elseif(yaml-cpp_FOUND)
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DHAS_YAMPCPP)
    target_link_libraries(${PROJECT_NAME} PUBLIC yaml-cpp)
endif()
if(CCI_FOUND)
    message(STATUS "${PROJECT_NAME}: building with external CCI: ${CCI_LIBRARIES}")
    target_compile_definitions(${PROJECT_NAME} PUBLIC ${CCI_DEFINITIONS})
    target_include_directories (${PROJECT_NAME} PUBLIC ${CCI_INCLUDE_DIRS})    
    target_link_libraries(${PROJECT_NAME} PUBLIC ${CCI_LIBRARIES} )
else()
    target_link_libraries(${PROJECT_NAME} PUBLIC systemc-cci::systemc-cci)
endif()
if(ENABLE_SCV AND TARGET SystemC::scv)
    message(STATUS "${PROJECT_NAME}: building with SCV support")
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DHAS_SCV)
    target_link_libraries(${PROJECT_NAME} PUBLIC SystemC::scv)
elseif(ENABLE_SCV AND SCV_FOUND)
    message(STATUS "${PROJECT_NAME}: building with SCV support")
    target_compile_definitions(${PROJECT_NAME} PUBLIC ${SCVDEFINITIONS} -DHAS_SCV)
    target_include_directories (${PROJECT_NAME} PUBLIC ${SCV_INCLUDE_DIRS})    
    target_link_libraries(${PROJECT_NAME} PUBLIC ${SCV_LIBRARIES} )
else()   
    message(STATUS "${PROJECT_NAME}: building with SCV-TR support")
    target_link_libraries (${PROJECT_NAME} PUBLIC scv-tr)
endif()
# make sure dynamic processes get includes since CCI uses them
target_compile_definitions(${PROJECT_NAME} PUBLIC SC_INCLUDE_DYNAMIC_PROCESSES)
if(TARGET SystemC::systemc)
    message(STATUS "${PROJECT_NAME}: link with SystemC::systemc")
    if(USE_CWR_SYSTEMC OR USE_NCSC_SYSTEMC)
        get_target_property(INCLS SystemC::systemc INTERFACE_INCLUDE_DIRECTORIES)
        get_target_property(DIRS SystemC::systemc INTERFACE_LINK_DIRECTORIES)
        get_target_property(LIBS SystemC::systemc INTERFACE_LINK_LIBRARIES)
        target_include_directories (${PROJECT_NAME} PUBLIC ${INCLS})
        target_link_directories(${PROJECT_NAME} PUBLIC ${DIRS})
        target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS})
    else()
        target_link_libraries(${PROJECT_NAME} PUBLIC SystemC::systemc)
    endif()
else()
    message(STATUS "${PROJECT_NAME}: link with ${SystemC_INCLUDE_DIRS}, ${SystemC_DEFINITIONS}, and ${SystemC_LIBRARIES}")
    target_compile_definitions(${PROJECT_NAME} PUBLIC ${SystemC_DEFINITIONS})
    target_include_directories (${PROJECT_NAME} PUBLIC ${SystemC_INCLUDE_DIRS})    
    target_link_libraries(${PROJECT_NAME} PUBLIC ${SystemC_LIBRARIES} )
endif()
if(ZLIB_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC ZLIB::ZLIB)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_ZLIB)
endif()
if(WITH_FST)
    target_compile_definitions(${PROJECT_NAME} PUBLIC WITH_FST)
endif()
if(TARGET lz4::lz4)
    target_link_libraries(${PROJECT_NAME} PRIVATE lz4::lz4)
endif()
target_compile_definitions(${PROJECT_NAME} PUBLIC FMT_SHARED)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" AND TARGET fmt::fmt-header-only)
	target_link_libraries(${PROJECT_NAME} PUBLIC fmt::fmt-header-only)
else()
    target_link_libraries(${PROJECT_NAME} PUBLIC fmt::fmt)
endif()
target_link_libraries(${PROJECT_NAME} PUBLIC fstapi Threads::Threads ${CMAKE_DL_LIBS})

if(CLANG_TIDY_EXE)
    set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
endif() 

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    FRAMEWORK FALSE
    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/scc_sysc.h
)

install(TARGETS ${PROJECT_NAME} COMPONENT sysc EXPORT ${PROJECT_NAME}-targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_LIBRARY_DIR_MODIFIER}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_LIBRARY_DIR_MODIFIER}${SCC_ARCHIVE_DIR_MODIFIER}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
        
install(EXPORT ${PROJECT_NAME}-targets
        DESTINATION ${SCC_CMAKE_CONFIG_DIR}
        NAMESPACE scc::
        )

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scc COMPONENT sysc
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.h" # select header files
        )
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scc COMPONENT sysc
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.hh" # select header files
        )
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tlm COMPONENT sysc
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.h" # select header files
        )

