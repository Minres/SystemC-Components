cmake_minimum_required(VERSION 3.12)

project(scv-tr LANGUAGES CXX VERSION 0.1.9)

include(GNUInstallDirs)

#enable_testing() 

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)

if(COMPILER_SUPPORTS_MARCH_NATIVE)
	if("${CMAKE_BUILD_TYPE}" STREQUAL "") 
	    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
	elseif(NOT(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo"))
	    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
	endif()
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(warnings "-Wall -Wextra -Werror")
    set(CMAKE_CXX_FLAG_RELEASE "-O3 -DNDEBUG")
    set(CMAKE_C_FLAG_RELEASE "-O3 -DNDEBUG")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(warnings "/W4 /WX /EHsc")
endif()

add_subdirectory(src)
