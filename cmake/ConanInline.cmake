
set(CONAN_CMAKE_LIST_DIR ${CMAKE_CURRENT_BINARY_DIR})

macro(conan_check)
  # for backwards compatibility
  cmake_parse_arguments(MARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
 
  find_program(CONAN conan)
  if(NOT EXISTS ${CONAN})
    message(FATAL_ERROR "Conan is required. Please see README.md")
    return()
  endif()
  execute_process(COMMAND ${CONAN} --version
                  OUTPUT_VARIABLE CONAN_VERSION_OUTPUT)
  string(REGEX MATCHALL "[0-9.]+" CONAN_VERSION ${CONAN_VERSION_OUTPUT})
  if (NOT (CONAN_VERSION VERSION_GREATER_EQUAL 1.36.0))
    message(FATAL_ERROR "Please upgrade your conan to a version greater or equal 1.36")
  endif()
 
  if(NOT EXISTS ${CONAN_CMAKE_LIST_DIR}/conan.cmake)
    message("Downloading conan.cmake to ${CONAN_CMAKE_LIST_DIR}")
    #set(URL https://raw.githubusercontent.com/conan-io/cmake-conan/develop/conan.cmake)
    set(URL https://raw.githubusercontent.com/conan-io/cmake-conan/43e385830ee35377dbd2dcbe8d5a9e750301ea00/conan.cmake)
    file(DOWNLOAD ${URL} ${CONAN_CMAKE_LIST_DIR}/conan.cmake TIMEOUT 60 STATUS DOWNLOAD_STATUS)
    list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
    list(GET DOWNLOAD_STATUS 1 ERROR_MESSAGE)
    if(NOT (${STATUS_CODE} EQUAL 0))
        # Exit CMake if the download failed, printing the error message.
        message(FATAL_ERROR "Error occurred during download: ${ERROR_MESSAGE}")
    endif()
    if(NOT EXISTS ${CONAN_CMAKE_LIST_DIR}/conan.cmake)
        message(FATAL_ERROR "Could not download conan.cmake. Please check your internet connection or proxy settings")
    endif()
    file (SIZE ${CONAN_CMAKE_LIST_DIR}/conan.cmake CONAN_CMAKE_SIZE)
    if(${CONAN_CMAKE_SIZE} EQUAL 0)
         message(FATAL_ERROR "Could not download conan.cmake. Please check your internet connection or proxy settings")
    endif()
  endif()
  if("${CMAKE_BUILD_TYPE}" STREQUAL "")
      set(CMAKE_BUILD_TYPE Release)
  endif()
  
  
  include(${CONAN_CMAKE_LIST_DIR}/conan.cmake)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_BINARY_DIR})
endmacro()

macro(conan_setup)
  set(options TARGETS) 
  cmake_parse_arguments(MARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
 

  set(conanfile_cmake_paths ${CMAKE_BINARY_DIR}/conan_paths.cmake)

  set(conanfile_cmake ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
  if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
	  set(conanfile_cmake ${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
  endif()

  if(EXISTS "${conanfile_cmake_paths}")
    include(${conanfile_cmake_paths})
  elseif(EXISTS "${conanfile_cmake}")
    include(${conanfile_cmake})
    if( MARGS_TARGETS)
      conan_basic_setup(TARGETS)
    else()
      conan_basic_setup()
    endif()
  endif()
endmacro()

function(conan_configure)
    conan_cmake_generate_conanfile(OFF ${ARGV})
endfunction()

macro(conan_install)
  	set(options BUILD_TYPE) 
 	set(oneValueArgs BUILD_TYPE)
	cmake_parse_arguments(MARGS "" "${oneValueArgs}" "" ${ARGN} )
  	if(MARGS_BUILD_TYPE)
		conan_cmake_autodetect(settings BUILD_TYPE ${MARGS_BUILD_TYPE})
  	else()
		conan_cmake_autodetect(settings BUILD_TYPE)
	endif()
	if(CMAKE_CXX_STANDARD)
		set(settings ${settings} compiler.cppstd=${CMAKE_CXX_STANDARD})
	endif()
	if (NOT "$ENV{CONAN_PROFILE_NAME}" STREQUAL "")
    	set(CONAN_PROFILE "$ENV{CONAN_PROFILE_NAME}" CACHE INTERNAL "Copied from environment variable")
    else()
    	set(CONAN_PROFILE "default" CACHE INTERNAL "Copied from environment variable")
   endif()
	
	conan_cmake_install(PATH_OR_REFERENCE .
	                    BUILD missing
	                    PROFILE ${CONAN_PROFILE}
	                    SETTINGS ${settings})
endmacro()
