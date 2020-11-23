cmake_minimum_required(VERSION 2.8)

set(APP_NAME axi_axi_test)

include_directories(${TLM_INCLUDE_DIRS} ${SystemC_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
find_library(TCMALLOC_LIB NAMES libtcmalloc.so)

###############################################################################

project (${APP_NAME})
link_directories(${SCV_LIBRARY_DIRS} ${SystemC_LIBRARY_DIRS})

add_executable(${APP_NAME} sc_main.cpp)

target_link_libraries (${APP_NAME} PUBLIC tlm2-interfaces)
#target_link_libraries (${APP_NAME} PUBLIC scc)
#target_link_libraries (${APP_NAME} PUBLIC scc-util)
#target_link_libraries (${APP_NAME} LINK_PUBLIC ${SystemC_LIBRARIES} )
#target_link_libraries (${APP_NAME} LINK_PUBLIC ${SCV_LIBRARIES} )
target_link_libraries (${APP_NAME} LINK_PUBLIC ${CMAKE_THREAD_LIBS_INIT})
if(NOT TCMALLOC_LIB STREQUAL TCMALLOC_LIB-NOTFOUND)
	target_link_libraries (${APP_NAME} LINK_PUBLIC ${TCMALLOC_LIB})
endif()
target_link_libraries (${APP_NAME} LINK_PUBLIC ${CMAKE_DL_LIBS})
