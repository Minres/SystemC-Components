SET(_COMMON_HINTS
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\SystemC\\2.3;SystemcHome]/include"
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

SET(_SYSTEMC_HINTS
  ${CONAN_INCLUDE_DIRS_SYSTEMC}
  ${CONAN_LIB_DIRS_SYSTEMC}
  ${_COMMON_HINTS}
  )

SET(_SCV_HINTS
  ${CONAN_INCLUDE_DIRS_SYSTEMCVERIFICATION}
  ${CONAN_LIB_DIRS_SYSTEMCVERIFICATION}
  $ENV{SCV_HOME}/include
  $ENV{SCV_HOME}/lib
  $ENV{SCV_HOME}/lib-linux
  $ENV{SCV_HOME}/lib-linux64
  $ENV{SCV_HOME}/lib-macos
  ${_COMMON_HINTS}
  )

SET(_CCI_HINTS
  ${CONAN_INCLUDE_DIRS_SYSTEMC-CCI}
  ${CONAN_LIB_DIRS_SYSTEMC-CCI}
  $ENV{CCI_HOME}/include
  $ENV{CCI_HOME}/lib
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

SET(_SYSTEMC_PATHS
  ${CONAN_INCLUDE_DIRS_SYSTEMC}
  ${CONAN_LIB_DIRS_SYSTEMC}
  ${_COMMON_PATHS}
  )

SET(_SCV_PATHS
  ${CONAN_INCLUDE_DIRS_SYSTEMCVERIFICATION}
  ${CONAN_LIB_DIRS_SYSTEMCVERIFICATION}
  ${_COMMON_PATHS}
  )

SET(_CCI_PATHS
  ${CONAN_INCLUDE_DIRS_SYSTEMC}
  ${CONAN_LIB_DIRS_SYSTEMC}
  ${_COMMON_PATHS}
  )
  
FIND_FILE(_SYSTEMC_HEADER_FILE
  NAMES systemc
  HINTS ${_SYSTEMC_HINTS}
  PATHS ${_SYSTEMC_PATHS}
  PATH_SUFFIXES sysc/kernel
)

FIND_FILE(_SCV_HEADER_FILE
  NAMES scv.h
  HINTS ${_SCV_HINTS}
  PATHS ${_SCV_PATHS}
  PATH_SUFFIXES sysc/kernel
)

FIND_FILE(_CCI_HEADER_FILE
  NAMES cci_configuration
  HINTS ${_CCI_HINTS}
  PATHS ${_CCI_PATHS}
  PATH_SUFFIXES sysc/kernel
)

if(NOT _SYSTEMC_HEADER_FILE STREQUAL _SYSTEMC_HEADER_FILE-NOTFOUND)
  set(SystemC_FOUND TRUE)
endif()

if(NOT _SCV_HEADER_FILE STREQUAL _SCV_HEADER_FILE-NOTFOUND)
  set(SCV_FOUND TRUE)
endif()

if(NOT _CCI_HEADER_FILE STREQUAL _CCI_HEADER_FILE-NOTFOUND)
  set(CCI_FOUND TRUE)
endif()

FIND_PATH(SystemC_INCLUDE_DIRS
  NAMES systemc.h
  HINTS ${_SYSTEMC_HINTS}
  PATHS ${_SYSTEMC_PATHS}
)

FIND_PATH(SystemC_LIBRARY_DIRS
  NAMES libsystemc.so libsystemc.a 
  HINTS ${_SYSTEMC_HINTS}
  PATHS ${_SYSTEMC_PATHS}
)

FIND_PATH(SCV_INCLUDE_DIRS
  NAMES scv.h
  HINTS ${_SCV_HINTS}
  PATHS ${_SCV_PATHS}
)

FIND_PATH(SCV_LIBRARY_DIRS
  NAMES libscv.a libscv.so
  HINTS ${_SCV_HINTS}
  PATHS ${_SCV_PATHS}
)

FIND_PATH(CCI_INCLUDE_DIRS
  NAMES cci_configuration
  HINTS ${_CCI_HINTS}
  PATHS ${_CCI_PATHS}
)

FIND_PATH(CCI_LIBRARY_DIRS
  NAMES libcciapi.a libcciapi.so
  HINTS ${_CCI_HINTS}
  PATHS ${_CCI_PATHS}
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
        if(CCI_FOUND)
            set(CCI_LIBRARIES cciapi)
            message(STATUS "CCI header files are taken from ${CCI_INCLUDE_DIRS}")
            message(STATUS "CCI library is taken from ${CCI_LIBRARY_DIRS}")
        endif()
endif(SystemC_FOUND)
