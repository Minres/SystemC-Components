SET(_COMMON_HINTS
  $ENV{VCS_HOME}/include/systemc231
  )

SET(_SYSTEMC_HINTS
  ${_COMMON_HINTS}
  )

SET(_TLM_HINTS
  $ENV{VCS_HOME}/include/systemc231/tlm
  ${_COMMON_HINTS}
  )

SET(_SCV_HINTS
  ${_COMMON_HINTS}
  )

SET(_CCI_HINTS
  ${_COMMON_HINTS}
  )

SET(_COMMON_PATHS
  $ENV{XCLM_PATH}/linux64/lib
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
  PATHS ${_COMMON_PATHS}
)

FIND_FILE(_SCV_HEADER_FILE
  NAMES scv.h
  HINTS ${_SCV_HINTS}
  PATHS ${_COMMON_PATHS}
)

FIND_FILE(_CCI_HEADER_FILE
  NAMES cci_configuration
  HINTS ${_CCI_HINTS}
  PATHS ${_COMMON_PATHS}
)

if(NOT _SYSTEMC_HEADER_FILE STREQUAL _SYSTEMC_HEADER_FILE-NOTFOUND)
  set(SystemC_FOUND TRUE)
endif()

if(ENABLE_SCV)
if(NOT _SCV_HEADER_FILE STREQUAL _SCV_HEADER_FILE-NOTFOUND)
  set(SCV_FOUND TRUE)
endif()
endif()

if(NOT _CCI_HEADER_FILE STREQUAL _CCI_HEADER_FILE-NOTFOUND)
  set(CCI_FOUND TRUE)
endif()

FIND_PATH(SystemC_INCLUDE_DIRS
  NAMES systemc
  HINTS ${_SYSTEMC_HINTS}
  PATHS ${_COMMON_PATHS}
)

FIND_PATH(TLM_INCLUDE_DIRS
  NAMES tlm
  HINTS ${_TLM_HINTS}
  PATHS ${_COMMON_PATHS}
)

FIND_PATH(SystemC_LIBRARY_DIRS
  NAMES libsystemc231-gcc6-64.a
  HINTS ${_SYSTEMC_HINTS}
  PATHS ${_COMMON_PATHS}
)

FIND_PATH(SCV_INCLUDE_DIRS
  NAMES scv.h
  HINTS ${_SCV_HINTS}
  PATHS ${_COMMON_PATHS}
)

FIND_PATH(SCV_LIBRARY_DIRS
  NAMES libscv.so 
  HINTS ${_SCV_HINTS}
  PATHS ${_COMMON_PATHS}
)

FIND_PATH(CCI_INCLUDE_DIRS
  NAMES cci_configuration
  HINTS ${_CCI_HINTS}
  PATHS ${_COMMON_PATHS}
)

FIND_PATH(CCI_LIBRARY_DIRS
  NAMES libxmsccci_sh.so
  HINTS ${_CCI_HINTS}
  PATHS ${_COMMON_PATHS}
)

if(SystemC_FOUND)
	#see https://gitlab.kitware.com/cmake/community/wikis/FAQ#how-do-i-use-a-different-compiler
   	set(SystemC_INCLUDE_DIRS ${TLM_INCLUDE_DIRS} ${SystemC_INCLUDE_DIRS})
    set(SystemC_LIBRARIES $ENV{VCS_HOME}/linux64/lib/libsystemc231-gcc6-64.a)
    if(SCV_FOUND)
        set(SCV_LIBRARIES ${SystemC_LIBRARIES})
    endif(SCV_FOUND)
    if(CCI_FOUND)
        set(CCI_LIBRARIES cciapi)
    endif()
endif(SystemC_FOUND)
