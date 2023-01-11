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
set(NCSC_LIBS systemc_sh xmscCoSim_sh xmscCoroutines_sh)
#set(NCSC_LIBS systemc_sh xmscCoroutines_sh ovm scBootstrap_sh)

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


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(XMSystemC
  REQUIRED_VARS
    ${LIB_VAR_NAMES}
    SystemC_INCLUDE_DIR
    TLM_INCLUDE_DIR
  VERSION_VAR SystemC_VERSION
)

if(XMSystemC_FOUND)
  set(SystemC_FOUND True)
  get_filename_component(NCSC_LIB_DIR ${systemc_sh_LIBRARY} DIRECTORY)
  set(SystemC_LIBRARY_DIRS ${NCSC_LIB_DIR})
  set(SystemC_LIBRARIES ${NCSC_LIBS} ${NCROOT_PATH}/tools/lib/64bit/libudm.so)     
  set(SystemC_INCLUDE_DIRS ${TLM_INCLUDE_DIR} ${SystemC_INCLUDE_DIR})
  set(SystemC_DEFINITIONS ${PC_SystemC_CFLAGS_OTHER} NCSC)
endif()

if(SystemC_FOUND AND NOT TARGET SystemC::systemc)
  add_library(SystemC::systemc UNKNOWN IMPORTED)
  set_target_properties(SystemC::systemc PROPERTIES
    IMPORTED_LOCATION ${systemc_sh_LIBRARY}
    INTERFACE_COMPILE_DEFINITIONS "${SystemC_DEFINITIONS}"
    # INTERFACE_COMPILE_OPTIONS "${SystemC_OPTIONS}"
    INTERFACE_INCLUDE_DIRECTORIES "${SystemC_INCLUDE_DIRS}"
    INTERFACE_LINK_DIRECTORIES "${SystemC_LIBRARY_DIRS}"
    INTERFACE_LINK_LIBRARIES "${SystemC_LIBRARIES}"
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

