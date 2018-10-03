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

if (COMMAND _find_verilator)
    return()
endif()

function(verilator_target_compile_options target)
    set(options "")

    if (CMAKE_SYSTEM_NAME MATCHES CYGWIN)
        list(APPEND options -std=gnu++11)
    else()
        list(APPEND options -std=c++11)
    endif()

    list(APPEND options -fPIC)

    if (LTO)
        list(APPEND options -flto)
    endif()

    if (LOGIC_WARNINGS_INTO_ERRORS)
        list(APPEND options -Werror)
    endif()

    if (CMAKE_BUILD_TYPE MATCHES "Release" OR NOT CMAKE_BUILD_TYPE)
        list(APPEND options
            -O2
            -s
            -DNDEBUG
            -fdata-sections
            -ffunction-sections
        )
    elseif (CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
        list(APPEND options
            -Os
            -DNDEBUG
            -fdata-sections
            -ffunction-sections
        )
    elseif (CMAKE_BUILD_TYPE MATCHES "Coverage")
        list(APPEND options
            -O0
            -g
            --coverage
        )
    endif()

    list(APPEND options
        -fstrict-aliasing
        -pedantic
        -Wall
        -Wcast-qual
        -Wcomments
        -Wconversion
        -Wctor-dtor-privacy
        -Wdisabled-optimization
        -Wendif-labels
        -Wenum-compare
        -Wfloat-equal
        -Wformat=2
        -Wformat-nonliteral
        -Winit-self
        -Winvalid-pch
        -Wlogical-op
        -Wmissing-declarations
        -Wmissing-include-dirs
        -Wno-long-long
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Woverloaded-virtual
        -Wpacked
        -Wparentheses
        -Wpointer-arith
        #-Wredundant-decls
        -Wshadow
        -Wsign-conversion
        -Wsign-promo
        -Wstack-protector
        -Wstrict-null-sentinel
        -Wstrict-overflow=2
        -Wsuggest-attribute=noreturn
        -Wswitch-default
        -Wswitch-enum
        -Wundef
        -Wuninitialized
        -Wunknown-pragmas
        -Wunused
        -Wunused-function
        -Wwrite-strings
    )

    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6.0)
        list(APPEND options
            -Wduplicated-cond
            -Whsa
            -Wignored-attributes
            -Wmisleading-indentation
            -Wnull-dereference
            -Wplacement-new=2
            -Wshift-negative-value
            -Wshift-overflow=2
            -Wvirtual-inheritance
        )
    endif()

    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
        list(APPEND options
            -Wdouble-promotion
            -Wsized-deallocation
            -Wsuggest-override
            -Wtrampolines
            -Wvector-operation-performance
            -Wzero-as-null-pointer-constant
        )
    endif()

    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
        list(APPEND options
            -Wconditionally-supported
            -Wdate-time
            -Weffc++
            -Wextra
            -Winline
            -Wopenmp-simd
        )
    endif()

    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
        list(APPEND options
            -Wpedantic
            -Wsuggest-attribute=format
        )
    endif()

    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.6)
        list(APPEND options
            -Wnoexcept
        )
    endif()

    target_compile_options(${target} PRIVATE ${options} ${ARGN})
endfunction()

function(_find_verilator)
    find_package(PackageHandleStandardArgs REQUIRED)
    find_package(SystemC)

    find_program(VERILATOR_EXECUTABLE verilator
        HINTS $ENV{VERILATOR_ROOT} /usr/local/bin
        PATH_SUFFIXES bin
        DOC "Path to the Verilator executable"
    )
   	if(VERILATOR_EXECUTABLE STREQUAL VERILATOR_EXECUTABLE-NOTFOUND)
    	message( FATAL_ERROR "verilator not found." )
	endif()
    

    find_program(VERILATOR_COVERAGE_EXECUTABLE verilator_coverage
        HINTS $ENV{VERILATOR_ROOT} /usr/local/bin
        PATH_SUFFIXES bin
        DOC "Path to the Verilator coverage executable"
    )

    get_filename_component(VERILATOR_EXECUTABLE_DIR ${VERILATOR_EXECUTABLE}
        DIRECTORY)

    find_path(VERILATOR_INCLUDE_DIR verilated.h
        HINTS $ENV{VERILATOR_ROOT} ${VERILATOR_EXECUTABLE_DIR}/.. /usr/local
        PATH_SUFFIXES include share/verilator/include
        DOC "Path to the Verilator headers"
    )
   	if(VERILATOR_INCLUDE_DIR STREQUAL VERILATOR_INCLUDE_DIR-NOTFOUND)
    	message( FATAL_ERROR "verilator include files not found." )
	endif()

    mark_as_advanced(VERILATOR_EXECUTABLE)
    mark_as_advanced(VERILATOR_COVERAGE_EXECUTABLE)
    mark_as_advanced(VERILATOR_INCLUDE_DIR)

    find_package_handle_standard_args(Verilator REQUIRED_VARS
        VERILATOR_EXECUTABLE VERILATOR_COVERAGE_EXECUTABLE VERILATOR_INCLUDE_DIR)

    if (WIN32)
        set(library_policy STATIC)
    else()
        set(library_policy SHARED)
    endif()

    add_library(verilated ${library_policy}
        ${VERILATOR_INCLUDE_DIR}/verilated.cpp
        ${VERILATOR_INCLUDE_DIR}/verilated_cov.cpp
        ${VERILATOR_INCLUDE_DIR}/verilated_dpi.cpp
        ${VERILATOR_INCLUDE_DIR}/verilated_vcd_c.cpp
        ${VERILATOR_INCLUDE_DIR}/verilated_vcd_sc.cpp
        #${CMAKE_CURRENT_LIST_DIR}/verilator_callbacks.cpp
    )

    set_target_properties(verilated PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    )

    set_source_files_properties(
        ${VERILATOR_INCLUDE_DIR}/verilated.cpp
        PROPERTIES
            COMPILE_DEFINITIONS "VL_USER_STOP;VL_USER_FINISH"
    )

    set_source_files_properties(
        ${VERILATOR_INCLUDE_DIR}/verilated_cov.cpp
        PROPERTIES
            COMPILE_DEFINITIONS "VM_COVERAGE=1"
    )

    target_link_libraries(verilated PRIVATE systemc)

    set_target_properties(verilated PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    )

    target_include_directories(verilated SYSTEM PRIVATE
        ${VERILATOR_INCLUDE_DIR}
        ${VERILATOR_INCLUDE_DIR}/vltstd
        ${SYSTEMC_INCLUDE_DIRS}
    )

    if (CMAKE_CXX_COMPILER_ID MATCHES GNU)
        verilator_target_compile_options(verilated PRIVATE
            -Wno-attributes
            -Wno-cast-qual
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
        verilator_target_compile_options(verilated PRIVATE -Wno-everything)
    endif()

    set(VERILATOR_FOUND ${VERILATOR_FOUND} PARENT_SCOPE)
    set(VERILATOR_EXECUTABLE "${VERILATOR_EXECUTABLE}" PARENT_SCOPE)
    set(VERILATOR_INCLUDE_DIR "${VERILATOR_INCLUDE_DIR}" PARENT_SCOPE)
    set(VERILATOR_COVERAGE_EXECUTABLE "${VERILATOR_COVERAGE_EXECUTABLE}"
        PARENT_SCOPE)
endfunction()

_find_verilator()
