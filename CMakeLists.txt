cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake) # project specific cmake dir

# Set the version number of your project here (format is MAJOR.MINOR.PATCHLEVEL - e.g. 1.0.0)
set(VERSION_MAJOR "0")
set(VERSION_MINOR "0")
set(VERSION_PATCH "1")
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

if(${CMAKE_VERSION} VERSION_LESS "3.0") 
    project(scc)
else()
    if(POLICY CMP0048)
      cmake_policy(SET CMP0048 NEW) # CMake 3.0.0
    endif()
    project(scc VERSION ${VERSION})
endif()


include(Common)

# build a separate SCC-UTIL library for the use in non-systemc based systems
add_library(scc-util STATIC src/io-redirector.cpp)
target_include_directories (scc-util PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/incl)
FILE(GLOB SccUtilHeaders incl/util/*.h)
set_target_properties(scc-util PROPERTIES
  VERSION ${VERSION}  # ${VERSION} was defined in the main CMakeLists.
  FRAMEWORK FALSE
  PUBLIC_HEADER "${SccUtilHeaders}" # specify the public headers
)


if(NOT SystemCPackage)
	set(SystemCPackage SystemC)
endif()

find_package(Threads)
find_package(ZLIB)
find_package(${SystemCPackage} REQUIRED)

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
    src/tracer_base.cpp
    src/tracer.cpp
)

if(CCI_FOUND)
    set(LIB_SOURCES ${LIB_SOURCES} 
	    src/configurable_tracer.cpp
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

# Define the scc library
add_library(${LIBRARY_NAME} ${LIB_SOURCES})
target_include_directories (${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/incl)
FILE(GLOB LibHeaders incl/*.h)
set_target_properties(${LIBRARY_NAME} PROPERTIES
  VERSION ${VERSION}  # ${VERSION} was defined in the main CMakeLists.
  FRAMEWORK FALSE
  PUBLIC_HEADER "${LibHeaders}" # specify the public headers
)

target_link_libraries(${LIBRARY_NAME} scc-util ${CMAKE_DL_LIBS})
if(${CMAKE_VERSION} VERSION_LESS "3.2.0")    
    target_link_libraries(${LIBRARY_NAME} ${CMAKE_THREAD_LIBS_INIT})
else()
    target_link_libraries(${LIBRARY_NAME} Threads::Threads)
endif()

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
add_library(jsoncpp STATIC src/jsoncpp.cpp)
target_include_directories (jsoncpp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/incl)
FILE(GLOB JsonLibHeaders incl/json/*.h)
set_target_properties(jsoncpp PROPERTIES
  VERSION ${VERSION}  # ${VERSION} was defined in the main CMakeLists.
  FRAMEWORK FALSE
  PUBLIC_HEADER "${JsonLibHeaders}" # specify the public headers
)

install(TARGETS jsoncpp
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static
        PUBLIC_HEADER DESTINATION include/json
        )
install(TARGETS scc-util
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static
        PUBLIC_HEADER DESTINATION include/util
        )
install(TARGETS ${LIBRARY_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static
        PUBLIC_HEADER DESTINATION include
        )
install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/incl/
    DESTINATION include
    FILES_MATCHING PATTERN "*")
