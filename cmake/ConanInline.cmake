
set(CONAN_CMAKE_LIST_DIR ${CMAKE_CURRENT_LIST_DIR})

macro(setup_conan)
  # for backwards compatibility
  set(options Release Debug RelWithDebInfo TARGETS) 
  set(oneValueArgs PROFILE)
  cmake_parse_arguments(MARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
 
  find_program(conan conan)
  if(NOT EXISTS ${conan})
    message(FATAL_ERROR "Conan is required. Please see README.md")
    return()
  endif()
  execute_process(COMMAND ${conan} --version
                  OUTPUT_VARIABLE CONAN_VERSION_OUTPUT)
  string(REGEX MATCHALL "[0-9.]+" CONAN_VERSION ${CONAN_VERSION_OUTPUT})
  #message(STATUS "Conan version is ${CONAN_VERSION}")
 
  if(NOT EXISTS ${CONAN_CMAKE_LIST_DIR}/conan.cmake)
    message("Downloading conan.cmake to ${CONAN_CMAKE_LIST_DIR}")  
    file(DOWNLOAD https://raw.githubusercontent.com/conan-io/cmake-conan/develop/conan.cmake ${CONAN_CMAKE_LIST_DIR}/conan.cmake
         TIMEOUT 60  # seconds
    )
  endif()
 
  include(${CONAN_CMAKE_LIST_DIR}/conan.cmake)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_BINARY_DIR})
endmacro()

function(conan_configure)
    conan_cmake_generate_conanfile(OFF ${ARGV})
endfunction()

macro(conan_install)
	conan_cmake_autodetect(settings)
	if(CMAKE_CXX_STANDARD)
		set(settings ${settings} compiler.cppstd=${CMAKE_CXX_STANDARD})
	endif()
	message("Conan settings are ${settings}")
	conan_cmake_install(PATH_OR_REFERENCE .
	                    BUILD missing
	                    REMOTE conan-center
	                    SETTINGS ${settings})
endmacro()
