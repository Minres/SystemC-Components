# Distributed under the OSI-approved Apache 2 License.  See accompanying file LICENSE file
#[=======================================================================[.rst:
FindOSCISystemC
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
``SystemC_LIBRARY``
  Libraries needed to link to SystemC.
``SCV_FOUND``
  True if the system has the SCV library.
``SCV_VERSION``
  The version of the SCV library which was found.
``SCV_INCLUDE_DIRS``
  Include directories needed to use SCV.
``SCV_LIBRARY``
  Libraries needed to link to SCV.
``CCI_FOUND``
  True if the system has the CCI library.
``CCI_VERSION``
  The version of the CCI library which was found.
``CCI_INCLUDE_DIRS``
  Include directories needed to use CCI.
``CCI_LIBRARY``
  Library needed to link to CCI.

#]=======================================================================]

SET(_COMMON_HINTS
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\SystemC\\2.3;SystemcHome]/include"
  ${SYSTEMC_PREFIX}/include
  ${SYSTEMC_PREFIX}/lib
  ${SYSTEMC_PREFIX}/lib64
  ${SYSTEMC_PREFIX}/lib-linux
  ${SYSTEMC_PREFIX}/lib-linux64
  ${SYSTEMC_PREFIX}/lib-macos
  $ENV{SYSTEMC_PREFIX}/include
  $ENV{SYSTEMC_PREFIX}/lib
  $ENV{SYSTEMC_PREFIX}/lib64
  $ENV{SYSTEMC_PREFIX}/lib-linux
  $ENV{SYSTEMC_PREFIX}/lib-linux64
  $ENV{SYSTEMC_PREFIX}/lib-macos
  $ENV{SYSTEMC_HOME}/include
  $ENV{SYSTEMC_HOME}/lib
  $ENV{SYSTEMC_HOME}/lib64
  $ENV{SYSTEMC_HOME}/lib-linux
  $ENV{SYSTEMC_HOME}/lib-linux64
  $ENV{SYSTEMC_HOME}/lib-macos
  ${CMAKE_INSTALL_PREFIX}/include
  ${CMAKE_INSTALL_PREFIX}/lib
  ${CMAKE_INSTALL_PREFIX}/lib64
  ${CMAKE_INSTALL_PREFIX}/lib-linux
  ${CMAKE_INSTALL_PREFIX}/lib-linux64
  ${CMAKE_INSTALL_PREFIX}/lib-macos
  ${SYSTEMC_INCL}
  ${SYSTEMC_LIB}
  $ENV{SYSTEMC_INCL}
  $ENV{SYSTEMC_LIB}
  )

SET(_SYSTEMC_HINTS
  ${CONAN_INCLUDE_DIRS_SYSTEMC}
  ${CONAN_LIB_DIRS_SYSTEMC}
  ${SystemC_LIB_DIRS}
  ${_COMMON_HINTS}
  )

SET(_SCV_HINTS
  ${CONAN_INCLUDE_DIRS_SYSTEMCVERIFICATION}
  ${CONAN_LIB_DIRS_SYSTEMCVERIFICATION}
  ${CONAN_INCLUDE_DIRS_SYSTEMC}
  ${CONAN_LIB_DIRS_SYSTEMC-SCV}
  ${SCV_PREFIX}/include
  ${SCV_PREFIX}/lib
  ${SCV_PREFIX}/lib64
  ${SCV_PREFIX}/lib-linux
  ${SCV_PREFIX}/lib-linux64
  ${SCV_PREFIX}/lib-macos
  $ENV{SCV_HOME}/include
  $ENV{SCV_HOME}/lib
  $ENV{SCV_HOME}/lib64
  $ENV{SCV_HOME}/lib-linux
  $ENV{SCV_HOME}/lib-linux64
  $ENV{SCV_HOME}/lib-macos
  ${_COMMON_HINTS}
  )

SET(_CCI_HINTS
  ${CONAN_INCLUDE_DIRS_SYSTEMC-CCI}
  ${CONAN_LIB_DIRS_SYSTEMC-CCI}
  ${systemc-cci_INCLUDE_DIR}
  ${systemc-cci_LIB_DIRS}
  ${CCI_PREFIX}/include
  ${CCI_PREFIX}/lib
  ${CCI_PREFIX}/lib64
  ${CCI_PREFIX}/lib-linux
  ${CCI_PREFIX}/lib-linux64
  ${CCI_PREFIX}/lib-macos
  $ENV{CCI_HOME}/include
  $ENV{CCI_HOME}/lib
  $ENV{CCI_HOME}/lib64
  $ENV{CCI_HOME}/lib-linux
  $ENV{CCI_HOME}/lib-linux64
  $ENV{CCI_HOME}/lib-macos
  ${_COMMON_HINTS}
  )

SET(_COMMON_PATHS
  /usr/include/systemc
  /usr/lib
  /usr/lib-linux
  /usr/lib-linux64
  /usr/lib-macos
  /usr/local/include/sysc
  /usr/local/lib
  /usr/local/lib-linux
  /usr/local/lib-linux64
  /usr/local/lib-macos
  )
  
if (NOT SystemC_INCLUDE_DIR)
  FIND_PATH(SystemC_INCLUDE_DIR
    NAMES systemc
    HINTS ${_SYSTEMC_HINTS}
    PATHS ${_COMMON_PATHS}
  )
endif()

FIND_LIBRARY(SystemC_LIBRARY
  NAMES systemc 
  HINTS ${_SYSTEMC_HINTS}
  PATHS ${_COMMON_PATHS}
)

mark_as_advanced(
  SystemC_INCLUDE_DIR
  SystemC_LIBRARY
)

if (SystemC_INCLUDE_DIR)
  file(STRINGS "${SystemC_INCLUDE_DIR}/sysc/kernel/sc_ver.h" version-file
    REGEX "#define[ \t]SC_VERSION_(MAJOR|MINOR|PATCH).*")
  if (NOT version-file)
    message(AUTHOR_WARNING "SystemC_INCLUDE_DIR found, but sc_ver.h is missing")
  endif()
  list(GET version-file 0 major-line)
  list(GET version-file 1 minor-line)
  list(GET version-file 2 patch-line)
  string(REGEX REPLACE "^#define[ \t]+SC_VERSION_MAJOR[ \t]+([0-9]+)$" "\\1" SC_VERSION_MAJOR ${major-line})
  string(REGEX REPLACE "^#define[ \t]+SC_VERSION_MINOR[ \t]+([0-9]+)$" "\\1" SC_VERSION_MINOR ${minor-line})
  string(REGEX REPLACE "^#define[ \t]+SC_VERSION_PATCH[ \t]+([0-9]+)$" "\\1" SC_VERSION_PATCH ${patch-line})
  set(SystemC_VERSION ${SC_VERSION_MAJOR}.${SC_VERSION_MINOR}.${SC_VERSION_PATCH} CACHE STRING "SystemC Version")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OSCISystemC
  REQUIRED_VARS
    SystemC_LIBRARY
    SystemC_INCLUDE_DIR
  VERSION_VAR SystemC_VERSION
)

if(OSCISystemC_FOUND)
  set(SystemC_FOUND ${OSCISystemC_FOUND})
  get_filename_component(SystemC_LIBRARY_DIRS ${SystemC_LIBRARY} DIRECTORY)
  set(SystemC_INCLUDE_DIRS ${SystemC_INCLUDE_DIR})
  set(SystemC_DEFINITIONS ${PC_SystemC_CFLAGS_OTHER})
endif()

if(SystemC_FOUND AND NOT TARGET SystemC::systemc)
  #message("Create target SystemC::systemc")
  add_library(SystemC::systemc UNKNOWN IMPORTED)
  set_target_properties(SystemC::systemc PROPERTIES
    IMPORTED_LOCATION "${SystemC_LIBRARY}"
    INTERFACE_LINK_DIRECTORIES ${SystemC_LIBRARY_DIRS}
    INTERFACE_COMPILE_OPTIONS "${SystemC_DEFINITIONS}"    
    INTERFACE_INCLUDE_DIRECTORIES "${SystemC_INCLUDE_DIRS}"
  )
endif()

FIND_PATH(SCV_INCLUDE_DIR
  NAMES scv.h
  HINTS ${_SCV_HINTS}
  PATHS ${_COMMON_PATHS}
)

FIND_LIBRARY(SCV_LIBRARY
  NAMES scv
  HINTS ${_SCV_HINTS}
  PATHS ${_COMMON_PATHS}
)

mark_as_advanced(
  SCV_INCLUDE_DIR
  SCV_LIBRARY
)

if(NOT SCV_INCLUDE_DIR MATCHES "SCV_INCLUDE_DIR-NOTFOUND")
    if(SCV_INCLUDE_DIR AND SCV_LIBRARY)
	    set(SCV_FOUND TRUE)
    endif()
	
	if(SCV_FOUND)
	  set(SCV_LIBRARIES ${SCV_LIBRARY})
  	  get_filename_component(SCV_LIBRARY_DIR ${SCV_LIBRARY} DIRECTORY )
	  set(SCV_INCLUDE_DIRS ${SCV_INCLUDE_DIR})
	  set(SCV_DEFINITIONS ${PC_SCV_CFLAGS_OTHER})
	endif()
	
	if(SCV_FOUND AND NOT TARGET SystemC::scv)
      #message("Create target SystemC::scv")
	  add_library(SystemC::scv UNKNOWN IMPORTED)
	  set_target_properties(SystemC::scv PROPERTIES
	    IMPORTED_LOCATION "${SCV_LIBRARY}"
	    INTERFACE_COMPILE_OPTIONS "${PC_SCV_CFLAGS_OTHER}"
	    INTERFACE_INCLUDE_DIRECTORIES "${SCV_INCLUDE_DIR}"
	  )
	endif()
endif()

FIND_PATH(CCI_INCLUDE_DIR
  NAMES cci_configuration
  HINTS ${_CCI_HINTS}
  PATHS ${_COMMON_PATHS}
)

FIND_LIBRARY(CCI_LIBRARY
  NAMES cciapi
  HINTS ${_CCI_HINTS}
  PATHS ${_COMMON_PATHS}
)

mark_as_advanced(
  CCI_INCLUDE_DIR
  CCI_LIBRARY
)

if(NOT CCI_INCLUDE_DIR MATCHES "CCI_INCLUDE_DIR-NOTFOUND")
	if(CCI_INCLUDE_DIR AND CCI_LIBRARY)
	   set(CCI_FOUND TRUE)
	endif()
	
	if(CCI_FOUND)
	  set(CCI_LIBRARIES ${CCI_LIBRARY})
  	  get_filename_component(CCI_LIBRARY_DIR ${CCI_LIBRARY} DIRECTORY )
	  set(CCI_INCLUDE_DIRS ${CCI_INCLUDE_DIR})
	  set(CCI_DEFINITIONS ${PC_CCI_CFLAGS_OTHER})
	endif()
	
	if(CCI_FOUND AND NOT TARGET SystemC::cci)
      #message("Create target SystemC::cci")
	  add_library(SystemC::cci UNKNOWN IMPORTED)
	  set_target_properties(SystemC::cci PROPERTIES
	    IMPORTED_LOCATION "${CCI_LIBRARY}"
	    INTERFACE_COMPILE_OPTIONS "${PC_CCI_CFLAGS_OTHER}"
	    INTERFACE_INCLUDE_DIRECTORIES "${CCI_INCLUDE_DIR}"
	  )
	endif()
endif()
