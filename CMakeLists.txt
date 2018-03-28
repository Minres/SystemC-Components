cmake_minimum_required(VERSION 3.3)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake) # main (top) cmake dir
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake) # project specific cmake dir

project (sc-components)

# Set the version number of your project here (format is MAJOR.MINOR.PATCHLEVEL - e.g. 1.0.0)
set(VERSION_MAJOR "0")
set(VERSION_MINOR "0")
set(VERSION_PATCH "1")
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

include(Common)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Threads)
find_package(SystemC REQUIRED)
find_package(ZLIB)
if(SystemC_FOUND)
    include_directories(${SystemC_INCLUDE_DIRS})
    link_directories(${SystemC_LIBRARY_DIRS})
else()
    message( FATAL_ERROR "SystemC library not found." )
endif()

if(CCI_FOUND)
    include_directories(${CCI_INCLUDE_DIRS})
    link_directories(${CCI_LIBRARY_DIRS})
else()
    message( FATAL_ERROR "SystemC CCI library not found." )
endif()

if(ZLIB_FOUND)
    include_directories( ${ZLIB_INCLUDE_DIRS} )
    #link_directories(${ZLIB_LIBRARY_DIRS})
    message( STATUS "ZLib found at ${ZLIB_INCLUDE_DIRS}." )
endif(ZLIB_FOUND)

if(SCV_FOUND)   
    add_definitions(-DWITH_SCV)
    include_directories(${SCV_INCLUDE_DIRS})
    link_directories(${SCV_LIBRARY_DIRS})
endif(SCV_FOUND)

add_subdirectory (src)
