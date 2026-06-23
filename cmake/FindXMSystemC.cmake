# Distributed under the OSI-approved Apache 2 License.  See accompanying file LICENSE file
#[=======================================================================[.rst:
FindXMSystemC
-------

Finds the SystemC and acompanying libraries.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``SystemC::systemc``
  The SystemC library
``SystemC::scv``
  The SystemC Verification library
``SystemC::cci``
  The SystemC CCI library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``SystemC_FOUND``
  True if the system has the SystemC library.
``SystemC_VERSION``
  The version of the SystemC library which was found.
``SystemC_INCLUDE_DIRS``
  Include directories needed to use SystemC.
``SystemC_LIBRARIES``
  Libraries needed to link to SystemC.
``SCV_FOUND``
  True if the system has the SCV library.
``SCV_VERSION``
  The version of the SCV library which was found.
``SCV_INCLUDE_DIRS``
  Include directories needed to use SCV.
``SCV_LIBRARIES``
  Libraries needed to link to SCV.

#]=======================================================================]
# find install location and version of XCelium
find_program(ncroot_EXECUTABLE ncroot) 
if(NOT ncroot_EXECUTABLE)
	message(FATAL_ERROR "No ncroot in PATH")
endif()

execute_process(
	COMMAND ${ncroot_EXECUTABLE}
	OUTPUT_VARIABLE CDSROOT_PATH
	)
string(STRIP ${CDSROOT_PATH} CDSROOT_PATH)

find_program(ncsc_EXECUTABLE ncsc) 
if(NOT ncsc_EXECUTABLE)
	message(FATAL_ERROR "No ncsc in PATH")
endif()

execute_process(
	COMMAND ${ncsc_EXECUTABLE} -version
	OUTPUT_VARIABLE NCSC_OUTPUT
	)
separate_arguments(NCSC_OUTPUT_LIST NATIVE_COMMAND ${NCSC_OUTPUT})
list(GET NCSC_OUTPUT_LIST 2 NCSC_VERSION_STRING)
string(SUBSTRING ${NCSC_VERSION_STRING} 0 2 NCSC_VERSION_MAJOR)
string(SUBSTRING ${NCSC_VERSION_STRING} 3 2 NCSC_VERSION_MINOR)
set(NCSC_VERSION ${NCSC_VERSION_MAJOR}${NCSC_VERSION_MINOR})
message(STATUS "Xcelium is at ${CDSROOT_PATH}, version is ${NCSC_VERSION_MAJOR}.${NCSC_VERSION_MINOR}")
# if we reach here we found Xcelium
set(XMSystemC_FOUND True)
# check if we build 32 or 64bit
file(WRITE "${CMAKE_BINARY_DIR}/test_arch_size.cpp" "
#include <cstdlib>
int main( int argc, char** argv ){
  return  sizeof(void*);
}")
set(CMAKE_TRY_COMPILE_CONFIGURATION  ${CMAKE_BUILD_TYPE})
TRY_RUN(RUN_RESULT_VAR COMPILE_RESULT_VAR
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/test_arch_size.cpp
  RUN_OUTPUT_VARIABLE IS_64_SYSTEM)
if(${RUN_RESULT_VAR} EQUAL 8)
  set(BITS_MOD 64bit)
  set(LIB_MOD 64)
endif()
# set default values of variables
set(NCSC_ROOT_PATH ${CDSROOT_PATH}/tools.lnx86/systemc)
set(NCSC_LIBS systemc_sh scBootstrap_sh xmscCoroutines_sh)
set(NCSC_LIB_PATHS ${CDSROOT_PATH}/tools.lnx86/systemc/lib/${BITS_MOD}/gnu)
set(GCC_LIB_PATH ${CDSROOT_PATH}/tools.lnx86/cdsgcc/gcc/install/lib${LIB_MOD})
# check if there is and will be used SystemC 3
if(NCSC_VERSION GREATER_EQUAL 2603 AND USE_NCSC_SYSTEMC3)
  set(SystemC_VERSION 3.0.1)
  set(NCSC_ROOT_PATH ${CDSROOT_PATH}/tools.lnx86/systemc_301)
  set(NCSC_LIB_PATHS ${CDSROOT_PATH}/tools.lnx86/systemc_301/lib/${BITS_MOD}/gnu)
else()
  set(SystemC_VERSION 2.3.4 CACHE STRING "SystemC Version")
endif()
# seed the include directories
set(NCSC_SystemC_INCLUDE_DIRS ${NCSC_ROOT_PATH}/include;${CDSROOT_PATH}/tools.lnx86/tbsc/include;${CDSROOT_PATH}/tools.lnx86/vic/include;${NCSC_ROOT_PATH}/include/factory;${NCSC_ROOT_PATH}/include/tlm2)
# find needed libraries
foreach(LIB_NAME ${NCSC_LIBS})
	FIND_LIBRARY(${LIB_NAME}_LIBRARY
	  NAMES ${LIB_NAME} 
	  HINTS ${NCSC_LIB_PATHS}
	  PATHS ${NCSC_LIB_PATHS}
	)
	list(APPEND LIB_FILE_NAMES ${${LIB_NAME}_LIBRARY})
	list(APPEND LIB_VAR_NAMES ${LIB_NAME}_LIBRARY)
endforeach()

mark_as_advanced(
  	NCSC_SystemC_INCLUDE_DIRS
  	NCSC_SystemC_LIBRARIES
	${LIB_VAR_NAMES}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(XMSystemC
  REQUIRED_VARS
    ${LIB_VAR_NAMES}
    NCSC_SystemC_INCLUDE_DIRS
  VERSION_VAR SystemC_VERSION
)

if(XMSystemC_FOUND)
  set(SystemC_FOUND True)
  get_filename_component(NCSC_LIB_DIR ${systemc_sh_LIBRARY} DIRECTORY)
  set(SystemC_LIBRARY_DIRS ${NCSC_LIB_DIR})
  set(SystemC_LIBRARIES ${NCSC_LIBS})     
  set(SystemC_INCLUDE_DIRS ${NCSC_SystemC_INCLUDE_DIRS})
  set(SystemC_DEFINITIONS NCSC)
endif()


if(SystemC_FOUND AND NOT TARGET SystemC::systemc)
  add_library(SystemC::systemc UNKNOWN IMPORTED)
  set_target_properties(SystemC::systemc PROPERTIES
    IMPORTED_LOCATION ${systemc_sh_LIBRARY}
    INTERFACE_COMPILE_DEFINITIONS "${SystemC_DEFINITIONS}"
    INTERFACE_INCLUDE_DIRECTORIES "${SystemC_INCLUDE_DIRS}"
    INTERFACE_LINK_DIRECTORIES "${SystemC_LIBRARY_DIRS};${GCC_LIB_PATH}"
    INTERFACE_LINK_LIBRARIES "${SystemC_LIBRARIES}"
  )
  if(UNIX)
    set_target_properties(SystemC::systemc PROPERTIES
      INTERFACE_LINK_OPTIONS "LINKER:-rpath,${NCSC_LIB_PATHS}:${GCC_LIB_PATH}"
    )
  if(NCSC_VERSION GREATER_EQUAL 2509)
    #target_compile_options(SystemC::systemc PUBLIC -Wno-free-nonheap-object)
    add_compile_options(-Wno-free-nonheap-object)
  endif()
  endif()
endif()
