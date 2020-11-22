cmake_minimum_required(VERSION 2.8)

set(APP_NAME axi_axi_test)
option(HAS_PCT "Enable Platform Architect" OFF)

if(HAS_PCT)
	if(HAS_SYSTEMC)
		set(ENABLE_SCV ON)
	endif()
	set(SystemCPackage SLSSystemC CACHE INTERNAL "" FORCE)
else()
	set(ENABLE_SCV ON)
	set(SystemCPackage OSCISystemC CACHE INTERNAL "" FORCE)
endif()

find_package(${SystemCPackage} REQUIRED)


set(Boost_NO_BOOST_CMAKE ON) #  Don't do a find_package in config mode before searching for a regular boost install.
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost REQUIRED COMPONENTS program_options)

# make sure the SLS include dirs precede the FlexNoC include dirs
include_directories(${TLM_INCLUDE_DIRS} ${SystemC_INCLUDE_DIRS})
# find_library(TCMALLOC_LIB NAMES libtcmalloc.so)

###############################################################################

project (${APP_NAME})
link_directories(${SCV_LIBRARY_DIRS} ${SystemC_LIBRARY_DIRS})

add_executable(${APP_NAME} sc_main.cpp)

target_include_directories (${APP_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../incl)
target_include_directories (${APP_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries (${APP_NAME} PUBLIC tlm2-interfaces)
target_link_libraries (${APP_NAME} PUBLIC scc)
target_link_libraries (${APP_NAME} PUBLIC scc-util)
target_link_libraries (${APP_NAME} PUBLIC )
target_link_libraries (${APP_NAME} LINK_PUBLIC ${SystemC_LIBRARIES} )
target_link_libraries (${APP_NAME} LINK_PUBLIC ${SCV_LIBRARIES} )
target_link_libraries (${APP_NAME} LINK_PUBLIC ${Boost_LIBRARIES} )
target_link_libraries (${APP_NAME} LINK_PUBLIC ${CMAKE_THREAD_LIBS_INIT})
if(NOT TCMALLOC_LIB STREQUAL TCMALLOC_LIB-NOTFOUND)
	target_link_libraries (${APP_NAME} LINK_PUBLIC ${TCMALLOC_LIB})
endif()
target_link_libraries (${APP_NAME} LINK_PUBLIC ${CMAKE_DL_LIBS})
