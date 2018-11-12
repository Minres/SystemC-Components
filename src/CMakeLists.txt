FILE(GLOB LibHeaders *.h)
set(LIB_HEADERS ${LibHeaders} )
set(LIB_SOURCES
    sc_logic_7.cpp
    tracer.cpp
    configurable_tracer.cpp
    utilities.cpp
    report.cpp
    jsoncpp.cpp
    configurer.cpp
    perf_estimator.cpp
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
add_library(${LIBRARY_NAME} ${LIB_SOURCES})
# Make sure the compiler can find include files for our library
# when other libraries or executables link to sc_components
target_include_directories (sc-components PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../incl)
