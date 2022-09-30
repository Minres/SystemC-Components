# Copyright 2018 Tymoteusz Blazejczyk
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#.rst:
# FindVerilator
# --------
#
# Find Verilator
#
# ::
#
#   VERILATOR_EXECUTABLE    - Verilator
#   VERILATOR_FOUND         - true if Verilator found

if (VERILATOR_FOUND)
    return()
endif()

find_package(PackageHandleStandardArgs REQUIRED)

include(SystemCPackage)
if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
endif()
find_program(VERILATOR_EXECUTABLE 
    NAMES verilator_bin verilator_bin_dbg
    HINTS ${CONAN_VERILATOR_ROOT} ENV VERILATOR_ROOT
    PATH_SUFFIXES bin
    REQUIRED
    DOC "Path to the Verilator executable"
)

if(${CONAN_VERILATOR_ROOT})
    set (ENV{VERILATOR_ROOT} ${CONAN_VERILATOR_ROOT}) 
    set (VERILATOR_EXECUTABLE_DIR ${CONAN_BIN_DIRS_VERILATOR})
    set (VERILATOR_INCLUDE_DIR ${CONAN_INCLUDE_DIRS_VERILATOR})
else()
    get_filename_component(VERILATOR_EXECUTABLE_DIR ${VERILATOR_EXECUTABLE}
        DIRECTORY)
    find_path(VERILATOR_INCLUDE_DIR verilated.h
        HINTS $ENV{VERILATOR_ROOT} ${VERILATOR_EXECUTABLE_DIR}/..
        PATH_SUFFIXES include share/verilator/include
        DOC "Path to the Verilator headers"
    )
endif()

mark_as_advanced(VERILATOR_EXECUTABLE)
mark_as_advanced(VERILATOR_INCLUDE_DIR)

find_package_handle_standard_args(Verilator REQUIRED_VARS
    VERILATOR_EXECUTABLE VERILATOR_INCLUDE_DIR)

set_source_files_properties(
    ${VERILATOR_INCLUDE_DIR}/verilated_cov.cpp
    PROPERTIES
    COMPILE_DEFINITIONS "VM_COVERAGE=1"
)

set(SOURCES 
    ${VERILATOR_INCLUDE_DIR}/verilated.cpp
    ${VERILATOR_INCLUDE_DIR}/verilated_cov.cpp
    # ${VERILATOR_INCLUDE_DIR}/verilated_threads.cpp
    ${VERILATOR_INCLUDE_DIR}/verilated_dpi.cpp
    ${VERILATOR_INCLUDE_DIR}/verilated_vcd_c.cpp
    ${VERILATOR_INCLUDE_DIR}/verilated_vcd_sc.cpp
)
if(EXISTS ${VERILATOR_INCLUDE_DIR}/verilated_fst_sc.cpp)
    list(APPEND SOURCES
        ${VERILATOR_INCLUDE_DIR}/verilated_fst_c.cpp
        ${VERILATOR_INCLUDE_DIR}/verilated_fst_sc.cpp
    )
endif()

add_library(verilated STATIC ${SOURCES})

set_target_properties(verilated PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

target_include_directories(verilated SYSTEM PUBLIC
    ${VERILATOR_INCLUDE_DIR}/vltstd
    ${VERILATOR_INCLUDE_DIR}
)

if (CMAKE_CXX_COMPILER_ID MATCHES GNU)
    target_compile_options(verilated PRIVATE
        -Wno-attributes
        -Wno-cast-qual
        -Wno-cast-equal
        -Wno-float-equal
        -Wno-suggest-override
        -Wno-conversion
        -Wno-unused-parameter
        -Wno-effc++
        -Wno-format
        -Wno-format-nonliteral
        -Wno-missing-declarations
        -Wno-old-style-cast
        -Wno-shadow
        -Wno-sign-conversion
        -Wno-strict-overflow
        -Wno-suggest-attribute=noreturn
        -Wno-suggest-final-methods
        -Wno-suggest-final-types
        -Wno-switch-default
        -Wno-switch-enum
        -Wno-undef
        -Wno-inline
        -Wno-variadic-macros
        -Wno-suggest-attribute=format
        -Wno-zero-as-null-pointer-constant
    )
elseif (CMAKE_CXX_COMPILER_ID MATCHES Clang)
    target_compile_options(verilated PRIVATE -Wno-everything)
endif()

if(SystemC_INCLUDE_DIRS)
    target_include_directories (verilated PUBLIC ${SystemC_INCLUDE_DIRS})    
    target_link_directories(verilated PUBLIC ${SystemC_LIBRARY_DIRS})
endif()
target_link_libraries(verilated PUBLIC ${SystemC_LIBRARIES} )


add_library(verilated_custom STATIC ${SOURCES})

set_target_properties(verilated_custom PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

target_include_directories(verilated_custom SYSTEM PUBLIC
    ${VERILATOR_INCLUDE_DIR}/vltstd
    ${VERILATOR_INCLUDE_DIR}
)

target_compile_definitions(verilated_custom PRIVATE VL_USER_FINISH VL_USER_STOP VL_USER_FATAL)
if (CMAKE_CXX_COMPILER_ID MATCHES GNU)
    target_compile_options(verilated_custom PRIVATE
        -Wno-attributes
        -Wno-cast-qual
        -Wno-cast-equal
        -Wno-float-equal
        -Wno-suggest-override
        -Wno-conversion
        -Wno-unused-parameter
        -Wno-effc++
        -Wno-format
        -Wno-format-nonliteral
        -Wno-missing-declarations
        -Wno-old-style-cast
        -Wno-shadow
        -Wno-sign-conversion
        -Wno-strict-overflow
        -Wno-suggest-attribute=noreturn
        -Wno-suggest-final-methods
        -Wno-suggest-final-types
        -Wno-switch-default
        -Wno-switch-enum
        -Wno-undef
        -Wno-inline
        -Wno-variadic-macros
        -Wno-suggest-attribute=format
        -Wno-zero-as-null-pointer-constant
    )
elseif (CMAKE_CXX_COMPILER_ID MATCHES Clang)
    target_compile_options(verilated_custom PRIVATE -Wno-everything)
endif()

if(SystemC_INCLUDE_DIRS)
    target_include_directories (verilated_custom PUBLIC ${SystemC_INCLUDE_DIRS})    
    target_link_directories(verilated_custom PUBLIC ${SystemC_LIBRARY_DIRS})
endif()
target_link_libraries(verilated_custom PUBLIC ${SystemC_LIBRARIES} )
