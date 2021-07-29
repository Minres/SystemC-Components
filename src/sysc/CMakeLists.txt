cmake_minimum_required(VERSION 3.12)

project(scc-sysc VERSION 0.0.1 LANGUAGES CXX)

find_package(Threads)
find_package(Boost REQUIRED COMPONENTS date_time filesystem)

option(ENABLE_SQLITE "Enable SQLite backend for SCV" OFF)

include(FindSystemCPackage)
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
    scc/scv/scv_tr_binary.cpp
    tlm/scc/scv/tlm_recorder.cpp
    tlm/scc/pe/parallel_pe.cpp
)

if(ZLIB_FOUND)
    set(LIB_SOURCES ${LIB_SOURCES} scc/scv/scv_tr_compressed.cpp)
endif()
if(ENABLE_SQLITE)
    set(LIB_SOURCES ${LIB_SOURCES} scc/scv/scv_tr_sqlite.cpp scc/scv/sqlite3.c )
endif()
if(CCI_FOUND)
    set(LIB_SOURCES ${LIB_SOURCES} 
        scc/configurable_tracer.cpp
        scc/sc_thread_pool.cpp
    )
endif()

add_library(${PROJECT_NAME} ${LIB_SOURCES})
target_include_directories (${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC scc-util jsoncpp ${CMAKE_DL_LIBS})
if(TARGET Boost::date_time)
	target_link_libraries(${PROJECT_NAME} PUBLIC Boost::date_time Boost::filesystem)
else()
	target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIRS})
	target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_datetime_LIBRARY} ${Boost_filesystem_LIBRARY})
endif()
if(SystemC_INCLUDE_DIRS)
    target_include_directories (${PROJECT_NAME} PUBLIC ${SystemC_INCLUDE_DIRS})    
    target_link_directories(${PROJECT_NAME} PUBLIC ${SystemC_LIBRARY_DIRS})
endif()
target_link_libraries(${PROJECT_NAME} PUBLIC ${SystemC_LIBRARIES} )
target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads)

if(ZLIB_FOUND)
    target_include_directories(${PROJECT_NAME} PUBLIC ${ZLIB_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${ZLIB_LIBRARIES})
    target_compile_definitions(${PROJECT_NAME} PRIVATE WITH_ZLIB)
endif()

if(CCI_FOUND)
	message("Building SCC with CCI support")
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DWITH_CCI)
    target_include_directories (${PROJECT_NAME} PUBLIC ${CCI_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${CCI_LIBRARIES} )
    if(NOT ${CMAKE_VERSION} VERSION_LESS "3.13.0") 
        target_link_directories(${PROJECT_NAME} PUBLIC ${CCI_LIBRARY_DIRS})
    endif()
endif()

if(SCV_FOUND)   
	message("Building SCC with SCV support")
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DWITH_SCV)
    target_include_directories (${PROJECT_NAME} PUBLIC ${SCV_INCLUDE_DIRS})
    target_link_libraries (${PROJECT_NAME} PUBLIC ${SCV_LIBRARIES})
    if(NOT ${CMAKE_VERSION} VERSION_LESS "3.13.0") 
        target_link_directories(${PROJECT_NAME} PUBLIC ${SCV_LIBRARY_DIRS})
    endif()
    if(ENABLE_SQLITE)
        target_compile_definitions(${PROJECT_NAME} PRIVATE WITH_SQLITE)
    endif()
else()
	message("Building SCC with SCV-TR support")
    target_link_libraries (${PROJECT_NAME} PUBLIC scv-tr)
    if(ENABLE_SQLITE)
        target_compile_definitions(${PROJECT_NAME} PRIVATE WITH_SQLITE)
    endif()
endif()

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
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
        
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scc COMPONENT sysc
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.h" # select header files
        )
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tlm COMPONENT sysc
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.h" # select header files
        )
