project(scc-sysc VERSION 0.0.1 LANGUAGES CXX)

find_package(Threads)
find_package(Boost REQUIRED COMPONENTS date_time filesystem)
find_package(ZLIB)

option(ENABLE_SQLITE "Enable SQLite backend for SCV" OFF)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif()

include(SystemCPackage)
if(SystemC_FOUND)
    message(STATUS "SystemC headers at ${SystemC_INCLUDE_DIRS}")
    message(STATUS "SystemC libraries: ${SystemC_LIBRARIES}")
    if(SCV_FOUND)
        message(STATUS "SCV headers at ${SCV_INCLUDE_DIRS}")
        message(STATUS "SCV libraries: ${SCV_LIBRARIES}")
    endif(SCV_FOUND)
    if(CCI_FOUND)
        message(STATUS "CCI headers at ${CCI_INCLUDE_DIRS}")
        message(STATUS "CCI libraries: ${CCI_LIBRARIES}")
    endif()
else()
    message(FATAL_ERROR "No usable SystemC installation found")
endif()

set(LIB_SOURCES
    scc/utilities.cpp 
    scc/configurer.cpp
    scc/tracer_base.cpp
    scc/tracer.cpp
    scc/perf_estimator.cpp
    scc/sc_logic_7.cpp
    scc/report.cpp
    scc/ordered_semaphore.cpp
    scc/value_registry.cpp
    scc/mt19937_rng.cpp
    scc/time_n_tick.cpp
    #scc/scv/scv_tr_binary.cpp
    scc/scv/scv_tr_mtc.cpp
    scc/scv/scv_tr_lz4.cpp
    scc/vcd_mt_trace.cpp
    scc/vcd_pull_trace.cpp
    scc/vcd_push_trace.cpp
    tlm/scc/scv/tlm_recorder.cpp
    tlm/scc/pe/parallel_pe.cpp
    scc/hierarchy_dumper.cpp
)

if(ZLIB_FOUND)
    message(STATUS "${PROJECT_NAME}: building zlib based parts")
    list(APPEND LIB_SOURCES  scc/scv/scv_tr_compressed.cpp)
	if(TARGET lz4::lz4 OR TARGET CONAN_PKG::lz4)
        list(APPEND LIB_SOURCES scc/fst_trace.cpp)
        set(WITH_FST ON)
    endif()
endif()

if(ENABLE_SQLITE)
    list(Append LIB_SOURCES  scc/scv/scv_tr_sqlite.cpp ../../third_party/sqlite3/sqlite3.c )
endif()

if(CCI_FOUND)
    list(APPEND LIB_SOURCES  
        scc/configurable_tracer.cpp
        scc/sc_thread_pool.cpp
    )
endif()

add_library(${PROJECT_NAME} ${LIB_SOURCES})
if(SCC_LIMIT_TRACE_TYPE_LIST)
    target_compile_definitions(${PROJECT_NAME} PRIVATE LIMIT_TRACE_TYPE_LIST)
endif()
if(SC_WITH_PHASE_CALLBACKS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE WITH_SC_PHASE_CALLBACKS)
endif()
if(SC_WITH_PHASE_CALLBACK_TRACING)
    target_compile_definitions(${PROJECT_NAME} PUBLIC WITH_SC_TRACING_PHASE_CALLBACKS)
endif()

target_include_directories (${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC scc-util RapidJSON ${SPDLOG_TARGET})
if(ENABLE_SQLITE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE WITH_SQLITE)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/sqlite3)
endif()
if(TARGET Boost::date_time)
    target_link_libraries(${PROJECT_NAME} PUBLIC Boost::date_time Boost::filesystem)
else()
    target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_datetime_LIBRARY} ${Boost_filesystem_LIBRARY})
endif()
if(TARGET SystemC::cci)
    message(STATUS "${PROJECT_NAME}: building with CCI support")
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DHAS_CCI)
    target_link_libraries(${PROJECT_NAME} PUBLIC SystemC::cci)
elseif(CCI_FOUND)
    message(STATUS "${PROJECT_NAME}: building with CCI support")
    target_compile_definitions(${PROJECT_NAME} PUBLIC ${CCI_DEFINITIONS} -DHAS_CCI)
    target_include_directories (${PROJECT_NAME} PUBLIC ${CCI_INCLUDE_DIRS})    
    target_link_libraries(${PROJECT_NAME} PUBLIC ${CCI_LIBRARIES} )
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
if(TARGET SystemC::systemc)
    message(STATUS "${PROJECT_NAME}: link with SystemC::systemc")
    if(USE_CWR_SYSTEMC)
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
    target_include_directories(${PROJECT_NAME} PUBLIC ${ZLIB_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${ZLIB_LIBRARIES})
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_ZLIB)
endif()
if(WITH_FST)
    target_compile_definitions(${PROJECT_NAME} PUBLIC WITH_FST)
    if(TARGET lz4::lz4)
    	target_link_libraries(${PROJECT_NAME} PRIVATE lz4::lz4)
    else()
    	target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::lz4)
    endif()
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC ${FMT_TARGET} fstapi Threads::Threads ${CMAKE_DL_LIBS})

if(CLANG_TIDY_EXE)
    set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
endif() 

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    FRAMEWORK FALSE
    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/scc_sysc.h
)

install(TARGETS ${PROJECT_NAME} COMPONENT sysc
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_ARCHIVE_DIR_MODIFIER}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
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
