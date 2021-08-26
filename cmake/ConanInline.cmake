
set(CONAN_CMAKE_LIST_DIR ${CMAKE_CURRENT_LIST_DIR})

macro(conan_check)
  # for backwards compatibility
  cmake_parse_arguments(MARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
 
  find_program(conan conan)
  if(NOT EXISTS ${conan})
    message(FATAL_ERROR "Conan is required. Please see README.md")
    return()
  endif()
  execute_process(COMMAND ${conan} --version
                  OUTPUT_VARIABLE CONAN_VERSION_OUTPUT)
  string(REGEX MATCHALL "[0-9.]+" CONAN_VERSION ${CONAN_VERSION_OUTPUT})
  if (NOT (CONAN_VERSION VERSION_GREATER_EQUAL 1.36.0))
    message(FATAL_ERROR "Please upgrade your conan to a version greater or equal 1.36")
  endif()
 
  if(NOT EXISTS ${CONAN_CMAKE_LIST_DIR}/conan.cmake)
    message("Downloading conan.cmake to ${CONAN_CMAKE_LIST_DIR}")  
    file(DOWNLOAD https://raw.githubusercontent.com/conan-io/cmake-conan/develop/conan.cmake ${CONAN_CMAKE_LIST_DIR}/conan.cmake
         TIMEOUT 60  # seconds
    )
  endif()
  if("${CMAKE_BUILD_TYPE}" STREQUAL "")
      set(CMAKE_BUILD_TYPE Release)
  endif()
  
  
  include(${CONAN_CMAKE_LIST_DIR}/conan.cmake)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_BINARY_DIR})
endmacro()

macro(conan_setup)
  set(options Release Debug RelWithDebInfo TARGETS) 
  set(oneValueArgs PROFILE)
  cmake_parse_arguments(MARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
 

  set(conanfile_cmake ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
  set(conanfile_cmake_paths ${CMAKE_BINARY_DIR}/conan_paths.cmake)

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
	conan_cmake_autodetect(settings)
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
