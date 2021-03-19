cmake_minimum_required(VERSION 3.12)

project(sysc VERSION 0.0.1 LANGUAGES CXX)

find_package(Threads)

if(NOT SystemCPackage)
    if(TARGET SystemC::SystemC)
        set(SystemCPackage SystemC)
    else()
        set(SystemCPackage OSCISystemC)
    endif()
endif()

if(TARGET SystemCVerification::SystemCVerification)
    set(SCV_FOUND TRUE)
    set(SCV_LIBRARIES SystemCVerification::SystemCVerification)
endif()
if(TARGET SystemC-CCI::SystemC-CCI)
    set(CCI_FOUND TRUE)
    set(CCI_LIBRARIES SystemC-CCI::SystemC-CCI)
endif()

message("Using ${SystemCPackage} package")
find_package(${SystemCPackage} REQUIRED)
if(SystemC_FOUND)
        message(STATUS "SystemC headers at ${SystemC_INCLUDE_DIRS}")
        message(STATUS "SystemC library at ${SystemC_LIBRARY_DIRS}")
        if(SCV_FOUND)
            message(STATUS "SCV headers at ${SCV_INCLUDE_DIRS}")
            message(STATUS "SCV library at ${SCV_LIBRARY_DIRS}")
        endif(SCV_FOUND)
        if(CCI_FOUND)
            message(STATUS "CCI headers at ${CCI_INCLUDE_DIRS}")
            message(STATUS "CCI library at ${CCI_LIBRARY_DIRS}")
        endif()
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
    tlm/pe/parallel_pe.cpp
)

if(SCV_FOUND )
    if(ZLIB_FOUND)
        set(LIB_SOURCES ${LIB_SOURCES} scv/scv_tr_compressed.cpp)
    endif(ZLIB_FOUND)
    if(FALSE)
        set(LIB_SOURCES ${LIB_SOURCES} scv/scv_tr_binary/scv_tr_binary.cpp)
        if(CONAN_LEVELDB_ROOT)
            set(LIB_SOURCES ${LIB_SOURCES} scv/scv_tr_ldb/scv_tr_ldb.cpp )
        endif()
    endif()
    set(LIB_SOURCES ${LIB_SOURCES} scv/scv_tr_sqlite/scv_tr_sqlite.cpp scv/scv_tr_sqlite/sqlite3.c )
endif()
if(CCI_FOUND)
    set(LIB_SOURCES ${LIB_SOURCES} 
        scc/configurable_tracer.cpp
    )
endif()

add_library(sysc STATIC ${LIB_SOURCES})
target_include_directories (sysc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(sysc PRIVATE scc-util jsoncpp)

target_include_directories (sysc PUBLIC ${SystemC_INCLUDE_DIRS})    
target_link_libraries(sysc PUBLIC ${SystemC_LIBRARIES} )
target_link_libraries(sysc PUBLIC Threads::Threads)

if(ZLIB_FOUND)
    target_include_directories(sysc PUBLIC ${ZLIB_INCLUDE_DIRS})
    target_link_libraries(sysc PUBLIC ${ZLIB_LIBRARIES})
    target_compile_definitions(sysc PRIVATE WITH_ZLIB)
endif()

if(CCI_FOUND)
	message("Building SCC with CCI support")
    target_compile_definitions(sysc PUBLIC -DWITH_CCI)
    target_include_directories (sysc PUBLIC ${CCI_INCLUDE_DIRS})
    target_link_libraries(sysc PUBLIC ${CCI_LIBRARIES} )
    if(NOT ${CMAKE_VERSION} VERSION_LESS "3.13.0") 
        target_link_directories(sysc PUBLIC ${CCI_LIBRARY_DIRS})
    endif()
endif()

if(SCV_FOUND)   
	message("Building SCC with SCV support")
    target_compile_definitions(sysc PUBLIC -DWITH_SCV)
    target_include_directories (sysc PUBLIC ${SCV_INCLUDE_DIRS})
    target_link_libraries (sysc PUBLIC ${SCV_LIBRARIES})
    if(NOT ${CMAKE_VERSION} VERSION_LESS "3.13.0") 
        target_link_directories(sysc PUBLIC ${SCV_LIBRARY_DIRS})
    endif()
endif()

set_target_properties(sysc PROPERTIES
    FRAMEWORK FALSE
    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/*
)

install(TARGETS sysc
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sysc
        )