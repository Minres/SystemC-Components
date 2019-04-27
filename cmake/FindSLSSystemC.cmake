SET(_COMMON_HINTS
  $ENV{SNPS_VP_HOME}/common/include
  $ENV{SNPS_VP_HOME}/common/libso-$ENV{COWARE_CXX_COMPILER}
  )

SET(_SYSTEMC_HINTS
  ${_COMMON_HINTS}
  )

SET(_TLM_HINTS
  $ENV{SNPS_VP_HOME}/common/include/tlm
  ${_COMMON_HINTS}
  )

SET(_SCV_HINTS
  ${_COMMON_HINTS}
  )

SET(_CCI_HINTS
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
  NAMES libSnpsVP.so 
  HINTS ${_SYSTEMC_HINTS}
  PATHS ${_COMMON_PATHS}
)

FIND_PATH(SCV_INCLUDE_DIRS
  NAMES scv.h
  HINTS ${_SCV_HINTS}
  PATHS ${_COMMON_PATHS}
)

FIND_PATH(SCV_LIBRARY_DIRS
  NAMES libSnpsVP.so 
  HINTS ${_SCV_HINTS}
  PATHS ${_COMMON_PATHS}
)

FIND_PATH(CCI_INCLUDE_DIRS
  NAMES cci_configuration
  HINTS ${_CCI_HINTS}
  PATHS ${_COMMON_PATHS}
)

FIND_PATH(CCI_LIBRARY_DIRS
  NAMES libcciapi.a libcciapi.so
  HINTS ${_CCI_HINTS}
  PATHS ${_COMMON_PATHS}
)

if(SystemC_FOUND)
	#see https://gitlab.kitware.com/cmake/community/wikis/FAQ#how-do-i-use-a-different-compiler
   	#set(CMAKE_CXX_COMPILER $ENV{SNPS_VP_HOME}/common/bin/g++)
   	set(SystemC_INCLUDE_DIRS ${TLM_INCLUDE_DIRS} ${SystemC_INCLUDE_DIRS})
    set(SystemC_LIBRARIES SnpsVPExt SnpsVP tbb omniORB4 omniDynamic4 omnithread dwarf elf)     
    #-L${SNPS_VP_HOME}/IP_common/common_debug/lib/linux.gcc-4.8.3-64 
    #-lipdebug 
    #-L${SNPS_VP_HOME}/IP/SBLGCCI_BL/Internal/lib/linux.gcc-4.8.3-64 
    #-lsblgcci    
    if(SCV_FOUND)
        set(SCV_LIBRARIES ${SystemC_LIBRARIES})
    endif(SCV_FOUND)
    if(CCI_FOUND)
        set(CCI_LIBRARIES cciapi)
    endif()
endif(SystemC_FOUND)
