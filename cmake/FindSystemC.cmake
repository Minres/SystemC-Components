SET(_SYSTEMC_HINTS
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\SystemC\\2.2;SystemcHome]/include"
  ${SYSTEMC_PREFIX}/include
  ${SYSTEMC_PREFIX}/lib
  ${SYSTEMC_PREFIX}/lib-linux
  ${SYSTEMC_PREFIX}/lib-linux64
  ${SYSTEMC_PREFIX}/lib-macos
  $ENV{SYSTEMC_PREFIX}/include
  $ENV{SYSTEMC_PREFIX}/lib
  $ENV{SYSTEMC_PREFIX}/lib-linux
  $ENV{SYSTEMC_PREFIX}/lib-linux64
  $ENV{SYSTEMC_PREFIX}/lib-macos
  $ENV{SYSTEMC_HOME}/include
  $ENV{SYSTEMC_HOME}/lib
  $ENV{SYSTEMC_HOME}/lib-linux
  $ENV{SYSTEMC_HOME}/lib-linux64
  $ENV{SYSTEMC_HOME}/lib-macos
  ${CMAKE_INSTALL_PREFIX}/include
  ${CMAKE_INSTALL_PREFIX}/lib
  ${CMAKE_INSTALL_PREFIX}/lib-linux
  ${CMAKE_INSTALL_PREFIX}/lib-linux64
  ${CMAKE_INSTALL_PREFIX}/lib-macos
  )
SET(_SYSTEMC_PATHS
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
FIND_FILE(_SYSTEMC_HEADER_FILE
  NAMES systemc
  HINTS ${_SYSTEMC_HINTS}
  PATHS ${_SYSTEMC_PATHS}
  PATH_SUFFIXES sysc/kernel
)

FIND_FILE(_SCV_HEADER_FILE
  NAMES scv.h
  HINTS ${_SYSTEMC_HINTS}
  PATHS ${_SYSTEMC_PATHS}
  PATH_SUFFIXES sysc/kernel
)

if(NOT _SYSTEMC_HEADER_FILE STREQUAL _SYSTEMC_HEADER_FILE-NOTFOUND)
  set(SystemC_FOUND TRUE)
endif(NOT _SYSTEMC_HEADER_FILE STREQUAL _SYSTEMC_HEADER_FILE-NOTFOUND)

if(NOT _SCV_HEADER_FILE STREQUAL _SCV_HEADER_FILE-NOTFOUND)
  set(SCV_FOUND TRUE)
endif(NOT _SCV_HEADER_FILE STREQUAL _SCV_HEADER_FILE-NOTFOUND)

FIND_PATH(SystemC_INCLUDE_DIRS
  NAMES systemc.h
  HINTS ${_SYSTEMC_HINTS}
  PATHS ${_SYSTEMC_PATHS}
)

FIND_PATH(SystemC_LIBRARY_DIRS
  NAMES libsystemc.a
  HINTS ${_SYSTEMC_HINTS}
  PATHS ${_SYSTEMC_PATHS}
)

FIND_PATH(SCV_INCLUDE_DIRS
  NAMES scv.h
  HINTS ${_SYSTEMC_HINTS}
  PATHS ${_SYSTEMC_PATHS}
)

FIND_PATH(SCV_LIBRARY_DIRS
  NAMES libscv.a
  HINTS ${_SYSTEMC_HINTS}
  PATHS ${_SYSTEMC_PATHS}
)

if(SystemC_FOUND)
        set(SystemC_LIBRARIES systemc)
        message(STATUS "SystemC header files are taken from ${SystemC_INCLUDE_DIRS}")
        message(STATUS "SystemC library is taken from ${SystemC_LIBRARY_DIRS}")
        if(SCV_FOUND)
            set(SCV_LIBRARIES scv)
            message(STATUS "SCV header files are taken from ${SCV_INCLUDE_DIRS}")
            message(STATUS "SCV library is taken from ${SCV_LIBRARY_DIRS}")
        endif(SCV_FOUND)
endif(SystemC_FOUND)
