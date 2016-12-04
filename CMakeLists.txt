cmake_minimum_required (VERSION 2.8.12)
project (SC-Components)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Threads)
find_package(SystemC)

if(SystemC_FOUND)
    include_directories(${SystemC_INCLUDE_DIRS})
    link_directories(${SystemC_LIBRARY_DIRS}
)
else()
    message( FATAL_ERROR "SystemC library not found." )
endif()

if(SCV_FOUND)   
    add_definitions(-DWITH_SCV)
    link_directories(${SCV_LIBRARY_DIRS})
    add_subdirectory (src)
    add_subdirectory (example)
endif(SCV_FOUND)