# Distributed under the OSI-approved Apache 2 License.  See accompanying file LICENSE file
#[=======================================================================[.rst:
FindMTISystemC
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
#]=======================================================================]
# find QuestSIM root installation
find_program(sccom_EXECUTABLE sccom) 
if(NOT sccom_EXECUTABLE)
	message(FATAL_ERROR "No sccom in PATH")
endif() 
get_filename_component(SCCOM_DIR ${sccom_EXECUTABLE} DIRECTORY)
get_filename_component(QUESTA_ROOT ${SCCOM_DIR} PATH)

# detect compiler version to be reflected in library name
if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
	string(REGEX MATCHALL "[0-9]+" GCC_VERSION_COMPONENTS ${CMAKE_CXX_COMPILER_VERSION})
	list(GET GCC_VERSION_COMPONENTS 0 GCC_MAJOR)
	list(GET GCC_VERSION_COMPONENTS 1 GCC_MINOR)
	set(COMPILER_VERSION "gcc${GCC_MAJOR}${GCC_MINOR}")
else()
	message(FATAL_ERROR "Compiler ${CMAKE_CXX_COMPILER_ID} is not supported")
endif()

SET(_COMMON_HINTS
  ${QUESTA_ROOT}/linux_x86_64
  )

SET(_SYSTEMC_HINTS
  ${QUESTA_ROOT}/include/systemc
  ${_COMMON_HINTS}
  )

SET(_SCV_HINTS
  ${QUESTA_ROOT}/include/scv
  ${_COMMON_HINTS}
  )

SET(_COMMON_PATHS
  )
  
FIND_PATH(SystemC_INCLUDE_DIR
  NAMES systemc
  HINTS ${_SYSTEMC_HINTS}
  PATHS ${_COMMON_PATHS}
)

FIND_LIBRARY(SystemC_LIBRARY
  NAMES systemc_${COMPILER_VERSION}
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
find_package_handle_standard_args(MTISystemC
  REQUIRED_VARS
    SystemC_LIBRARY
    SystemC_INCLUDE_DIR
  VERSION_VAR SystemC_VERSION
)

if(MTISystemC_FOUND)
  set(SystemC_FOUND ${MTISystemC_FOUND})
  get_filename_component(SystemC_LIBRARY_DIRS ${SystemC_LIBRARY} DIRECTORY)
  set(SystemC_INCLUDE_DIRS ${SystemC_INCLUDE_DIR} ${QUESTA_ROOT}/include)
  set(SystemC_DEFINITIONS DONT_USE_MTI_EXIT)
endif()

if(SystemC_FOUND AND NOT TARGET SystemC::systemc)
  add_library(SystemC::systemc UNKNOWN IMPORTED GLOBAL)
  set_target_properties(SystemC::systemc PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${SystemC_INCLUDE_DIRS}"
    INTERFACE_LINK_DIRECTORIES ${SystemC_LIBRARY_DIRS}
    INTERFACE_LINK_LIBRARIES "${SystemC_LIBRARY}"
    INTERFACE_COMPILE_DEFINITIONS "${SystemC_DEFINITIONS}"
    # INTERFACE_COMPILE_OPTIONS "${SystemC_systemc_COMPILE_OPTIONS_C};${SystemC_systemc_COMPILE_OPTIONS_CXX}"
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
