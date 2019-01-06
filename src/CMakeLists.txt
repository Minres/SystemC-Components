FILE(GLOB LibHeaders ../incl/*/*.h)
set(LIB_HEADERS ${LibHeaders} )
set(LIB_SOURCES
    sc_logic_7.cpp
    tracer_base.cpp
    tracer.cpp
    configurable_tracer.cpp
    utilities.cpp
    report.cpp
    jsoncpp.cpp
    configurer.cpp
    perf_estimator.cpp
    value_registry.cpp
    io-redirector.cpp
)

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
if(ENABLE_SHARED)
	add_library(${LIBRARY_NAME} SHARED ${LIB_SOURCES})
	include_directories(${SystemC_INCLUDE_DIRS})	
	include_directories(${CCI_INCLUDE_DIRS})
	if(SCV_FOUND)   
	    add_definitions(-DWITH_SCV)
	    include_directories(${SCV_INCLUDE_DIRS})
	endif()
	set_target_properties(${LIBRARY_NAME} PROPERTIES
	  VERSION ${VERSION}  # ${VERSION} was defined in the main CMakeLists.
	  FRAMEWORK FALSE
	  PUBLIC_HEADER "${LIB_HEADERS}" # specify the public headers
	)
	target_link_libraries(${LIBRARY_NAME} ${CCI_LIBRARIES} )
	if(SCV_FOUND)
	    link_directories(${LIBRARY_NAME})
	    target_link_libraries (${LIBRARY_NAME} ${SCV_LIBRARIES})
	endif()
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
else()
	add_library(${LIBRARY_NAME} ${LIB_SOURCES})
endif()
target_include_directories (sc-components PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../incl)

