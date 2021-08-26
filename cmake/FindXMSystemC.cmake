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
``CCI_FOUND``
  True if the system has the CCI library.
``CCI_VERSION``
  The version of the CCI library which was found.
``CCI_INCLUDE_DIRS``
  Include directories needed to use CCI.
``CCI_LIBRARIES``
  Libraries needed to link to CCI.

#]=======================================================================]
set(NCSC_LIBS systemc_sh scBootstrap_sh xmscCoroutines_sh ovm)

find_program(ncroot_EXECUTABLE ncroot) 
if(NOT ncroot_EXECUTABLE)
	message(FATAL_ERROR "No ncroot in PATH")
endif()

execute_process(
	COMMAND ${ncroot_EXECUTABLE}
	OUTPUT_VARIABLE NCROOT_PATH
	)
string(STRIP ${NCROOT_PATH} NCROOT_PATH)
message("Using Xcelium at ${NCROOT_PATH}")


SET(_COMMON_HINTS
	${NCROOT_PATH}/tools/systemc/include
	${NCROOT_PATH}/tools/systemc/lib/64bit
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
  	)

SET(_SYSTEMC_HINTS
  	${_COMMON_HINTS}
    )

SET(_TLM_HINTS
 	${NCROOT_PATH}/tools/systemc/include/tlm2
  	${_COMMON_HINTS}
    )

SET(_SCV_HINTS
  	${_COMMON_HINTS}
    )

SET(_CCI_HINTS
	${NCROOT_PATH}/tools/systemc/include/cci/
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
	${NCROOT_PATH}/tools/systemc/include
	${NCROOT_PATH}/tools/systemc/lib
  	/usr/include
  	/usr/lib
  	)
  
FIND_PATH(SystemC_INCLUDE_DIR
  NAMES systemc
  HINTS ${_SYSTEMC_HINTS}
  PATHS ${_COMMON_PATHS}
)

FIND_PATH(TLM_INCLUDE_DIR
  NAMES tlm
  HINTS ${_TLM_HINTS}
  PATHS ${_COMMON_PATHS}
)

foreach(LIB_NAME ${NCSC_LIBS})
	FIND_LIBRARY(${LIB_NAME}_LIBRARY
	  NAMES ${LIB_NAME} 
	  HINTS ${_SYSTEMC_HINTS}
	  PATHS ${_COMMON_PATHS}
	)
	set(LIB_FILE_NAMES ${LIB_FILE_NAMES} ${${LIB_NAME}_LIBRARY})
	set(LIB_VAR_NAMES ${LIB_VAR_NAMES} ${LIB_NAME}_LIBRARY)
endforeach()

mark_as_advanced(
  	SystemC_INCLUDE_DIR
  	SystemC_LIBRARY
	${LIB_VAR_NAMES}
)

if (SystemC_INCLUDE_DIR)
  file(STRINGS "${SystemC_INCLUDE_DIR}/sysc/cosim/xmsc_ver.h" version-file
    REGEX "#define[ \t]SC_VERSION_(MAJOR|MINOR|PATCH).*")
  if (NOT version-file)
    message(AUTHOR_WARNING "SystemC_INCLUDE_DIR found, but xmsc_ver.h is missing")
  endif()
  list(GET version-file 0 major-line)
  list(GET version-file 1 minor-line)
  list(GET version-file 2 patch-line)
  string(REGEX REPLACE "^#define[ \t]+SC_VERSION_MAJOR[ \t]+([0-9]+)$" "\\1" SC_VERSION_MAJOR ${major-line})
  string(REGEX REPLACE "^#define[ \t]+SC_VERSION_MINOR[ \t]+([0-9]+)$" "\\1" SC_VERSION_MINOR ${minor-line})
  string(REGEX REPLACE "^#define[ \t]+SC_VERSION_PATCH[ \t]+([0-9]+)$" "\\1" SC_VERSION_PATCH ${patch-line})
  set(SystemC_VERSION ${SC_VERSION_MAJOR}.${SC_VERSION_MINOR}.${SC_VERSION_PATCH} CACHE STRING "SystemC Version")
endif()


find_package_handle_standard_args(SystemC
  FOUND_VAR SystemC_FOUND
  REQUIRED_VARS
    ${LIB_VAR_NAMES}
    SystemC_INCLUDE_DIR
    TLM_INCLUDE_DIR
  VERSION_VAR SystemC_VERSION
)

if(SystemC_FOUND)
  get_filename_component(NCSC_LIB_DIR ${systemc_sh_LIBRARY} DIRECTORY)
  set(SystemC_LIBRARY_DIRS ${NCSC_LIB_DIR})
  set(SystemC_LIBRARIES ${NCSC_LIBS})     
  set(SystemC_INCLUDE_DIRS ${TLM_INCLUDE_DIR} ${SystemC_INCLUDE_DIR})
  set(SystemC_DEFINITIONS ${PC_SystemC_CFLAGS_OTHER} -DNCSC)
endif()

if(SystemC_FOUND AND NOT TARGET SystemC::systemc)
  add_library(SystemC::systemc UNKNOWN IMPORTED)
  set_target_properties(SystemC::systemc PROPERTIES
    IMPORTED_LOCATION ${systemc_sh_LIBRARY}
    INTERFACE_LINK_LIBRARIES "${SystemC_LIBRARIES}"
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
	find_package_handle_standard_args(SCV
	  FOUND_VAR SCV_FOUND
	  REQUIRED_VARS
	    SCV_LIBRARY
	    SCV_INCLUDE_DIR
	  VERSION_VAR 2.0.1
	)
	
	if(SCV_FOUND)
	  set(SCV_LIBRARIES ${SCV_LIBRARY})
	  set(SCV_INCLUDE_DIRS ${SCV_INCLUDE_DIR})
	  set(SCV_DEFINITIONS ${PC_SCV_CFLAGS_OTHER})
	endif()
	
	if(SCV_FOUND AND NOT TARGET SystemC::scv)
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
  NAMES xmsccci_sh
  HINTS ${_CCI_HINTS}
  PATHS ${_COMMON_PATHS}
)

mark_as_advanced(
  CCI_INCLUDE_DIR
  CCI_LIBRARY
)

if(NOT CCI_INCLUDE_DIR MATCHES "CCI_INCLUDE_DIR-NOTFOUND")
	find_package_handle_standard_args(CCI
	  FOUND_VAR CCI_FOUND
	  REQUIRED_VARS
	    CCI_LIBRARY
	    CCI_INCLUDE_DIR
	  VERSION_VAR 1.0.0
	)
	
	if(CCI_FOUND)
	  set(CCI_LIBRARIES ${CCI_LIBRARY})
	  set(CCI_INCLUDE_DIRS ${CCI_INCLUDE_DIR})
	  set(CCI_DEFINITIONS ${PC_CCI_CFLAGS_OTHER})
	endif()
	
	if(CCI_FOUND AND NOT TARGET SystemC::cci)
	  add_library(SystemC::cci UNKNOWN IMPORTED)
	  set_target_properties(SystemC::cci PROPERTIES
	    IMPORTED_LOCATION "${CCI_LIBRARY}"
	    INTERFACE_COMPILE_OPTIONS "${PC_CCI_CFLAGS_OTHER}"
	    INTERFACE_INCLUDE_DIRECTORIES "${CCI_INCLUDE_DIR}"
	  )
	endif()
endif()

