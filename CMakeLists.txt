cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake) # project specific cmake dir

project (scc)

# Set the version number of your project here (format is MAJOR.MINOR.PATCHLEVEL - e.g. 1.0.0)
set(VERSION_MAJOR "0")
set(VERSION_MINOR "0")
set(VERSION_PATCH "1")
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

include(Common)

if(NOT SystemCPackage)
	set(SystemCPackage SystemC)
endif()

find_package(Threads)
find_package(ZLIB)
find_package(${SystemCPackage} REQUIRED)

FILE(GLOB LibHeaders incl/*.h incl/*/*.h)
set(LIB_HEADERS ${LibHeaders} )

set(LIB_SOURCES
    src/sc_logic_7.cpp
    src/utilities.cpp
    src/report.cpp
    src/perf_estimator.cpp
    src/io-redirector.cpp
    src/ordered_semaphore.cpp
)

if(CCI_FOUND)
    set(LIB_SOURCES ${LIB_SOURCES} 
	    src/tracer_base.cpp
	    src/tracer.cpp
	    src/configurable_tracer.cpp
	    src/configurer.cpp
    	src/jsoncpp.cpp
    	src/value_registry.cpp
    )
endif()

if(SCV_FOUND)
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
endif(SCV_FOUND)

# Define two variables in order not to repeat ourselves.
set(LIBRARY_NAME scc)
if(${CMAKE_VERSION} VERSION_LESS "3.13.0") 
	link_directories(${SystemC_LIBRARY_DIRS})
endif()

# Define the library
add_library(${LIBRARY_NAME} ${LIB_SOURCES})

target_include_directories (${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/incl)
set_target_properties(${LIBRARY_NAME} PROPERTIES
  VERSION ${VERSION}  # ${VERSION} was defined in the main CMakeLists.
  FRAMEWORK FALSE
  PUBLIC_HEADER "${LIB_HEADERS}" # specify the public headers
)

if(CCI_FOUND)
    target_compile_definitions(${LIBRARY_NAME} PUBLIC -DWITH_CCI)
	target_include_directories (${LIBRARY_NAME} PUBLIC ${CCI_INCLUDE_DIRS})
	target_link_libraries(${LIBRARY_NAME} ${CCI_LIBRARIES} )
	if(NOT ${CMAKE_VERSION} VERSION_LESS "3.13.0") 
		target_link_directories(${LIBRARY_NAME} PUBLIC ${CCI_LIBRARY_DIRS})
	endif()
endif()

if(SCV_FOUND)   
    target_compile_definitions(${LIBRARY_NAME} PUBLIC -DWITH_SCV)
	target_include_directories (${LIBRARY_NAME} PUBLIC ${SCV_INCLUDE_DIRS})
    target_link_libraries (${LIBRARY_NAME} ${SCV_LIBRARIES})
	if(NOT ${CMAKE_VERSION} VERSION_LESS "3.13.0") 
		target_link_directories(${LIBRARY_NAME} PUBLIC ${SCV_LIBRARY_DIRS})
	endif()
endif()

if(ZLIB_FOUND)
    target_link_libraries (${LIBRARY_NAME} ${ZLIB_LIBRARIES})
endif(ZLIB_FOUND)

target_include_directories (${LIBRARY_NAME} PUBLIC ${SystemC_INCLUDE_DIRS})	
target_link_libraries(${LIBRARY_NAME} ${SystemC_LIBRARIES} )
if(NOT ${CMAKE_VERSION} VERSION_LESS "3.13.0") 
	target_link_directories(${LIBRARY_NAME} PUBLIC ${SystemC_LIBRARY_DIRS})
endif()

# build a separate JSON library for the use in non-systemc based systems
add_library(jsoncpp src/jsoncpp.cpp)
FILE(GLOB JsonLibHeaders incl/json/*.h)
set_target_properties(jsoncpp PROPERTIES
  VERSION ${VERSION}  # ${VERSION} was defined in the main CMakeLists.
  FRAMEWORK FALSE
  PUBLIC_HEADER "${JsonLibHeaders}" # specify the public headers
)
target_include_directories (jsoncpp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/incl)

install(TARGETS ${LIBRARY_NAME} jsoncpp
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static
        PUBLIC_HEADER DESTINATION include)
