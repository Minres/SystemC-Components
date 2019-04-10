FILE(GLOB LibHeaders ../incl/*/*.h)
set(LIB_HEADERS ${LibHeaders} )
set(LIB_SOURCES
    sc_logic_7.cpp
    utilities.cpp
    report.cpp
    perf_estimator.cpp
    io-redirector.cpp
)
if(CCI_FOUND)
    set(LIB_SOURCES ${LIB_SOURCES} 
	    tracer_base.cpp
	    tracer.cpp
	    configurable_tracer.cpp
	    configurer.cpp
    	jsoncpp.cpp
    	value_registry.cpp
    )
endif()

if(SCV_FOUND)
    if(ZLIB_FOUND)
        set(LIB_SOURCES ${LIB_SOURCES} scv_tr_compressed.cpp)
    endif(ZLIB_FOUND)
    if(FALSE)
    	set(LIB_SOURCES ${LIB_SOURCES} scv_tr_binary/scv_tr_binary.cpp)
	    if(CONAN_LEVELDB_ROOT)
		    set(LIB_SOURCES ${LIB_SOURCES} scv_tr_ldb/scv_tr_ldb.cpp )
	    endif()
    endif()
    set(LIB_SOURCES ${LIB_SOURCES} scv_tr_sqlite/scv_tr_sqlite.cpp scv_tr_sqlite/sqlite3.c )
endif(SCV_FOUND)

# Define two variables in order not to repeat ourselves.
set(LIBRARY_NAME sc-components)

# Define the library
add_library(${LIBRARY_NAME} ${LIB_SOURCES})
target_include_directories (${LIBRARY_NAME} PUBLIC ${SystemC_INCLUDE_DIRS})	
target_include_directories (${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../incl)
if(CCI_FOUND)
	target_include_directories (${LIBRARY_NAME} PUBLIC ${CCI_INCLUDE_DIRS})
	target_link_libraries(${LIBRARY_NAME} ${CCI_LIBRARIES} )
endif()
if(SCV_FOUND)   
    add_definitions(-DWITH_SCV)
	target_include_directories (${LIBRARY_NAME} PUBLIC ${SCV_INCLUDE_DIRS})
    target_link_libraries (${LIBRARY_NAME} ${SCV_LIBRARIES})
    target_link_libraries (${LIBRARY_NAME} fmt)
endif()
set_target_properties(${LIBRARY_NAME} PROPERTIES
  VERSION ${VERSION}  # ${VERSION} was defined in the main CMakeLists.
  FRAMEWORK FALSE
  PUBLIC_HEADER "${LIB_HEADERS}" # specify the public headers
)
target_link_libraries(${LIBRARY_NAME} ${SystemC_LIBRARIES} )

# build a separate JSON linrary for the use in non-systemc based systems
add_library(jsoncpp jsoncpp.cpp)
FILE(GLOB JsonLibHeaders ../incl/json/*.h)
set_target_properties(jsoncpp PROPERTIES
  VERSION ${VERSION}  # ${VERSION} was defined in the main CMakeLists.
  FRAMEWORK FALSE
  PUBLIC_HEADER "${JsonLibHeaders}" # specify the public headers
)
target_include_directories (jsoncpp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../incl)

