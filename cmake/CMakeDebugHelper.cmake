## https://stackoverflow.com/questions/32183975/how-to-print-all-the-properties-of-a-target-in-cmake/56738858#56738858
## https://stackoverflow.com/a/56738858/3743145

# Sets the AVAILABLE_CONFIGURATION_TYPES variable to the default available configurations
# (For some reason, CMAKE_CONFIGURATION_TYPES tends to be empty)
function(get_available_configuration_types)
    # Get all variables that cmake cache defines by default
    execute_process(COMMAND cmake -LAH -N OUTPUT_VARIABLE CMAKE_CACHE_VARIABLE_LIST)

    # Convert command output into a CMake list
    string(REGEX REPLACE ";" "[:semicolon:]" CMAKE_CACHE_VARIABLE_LIST "${CMAKE_CACHE_VARIABLE_LIST}")
    string(REGEX REPLACE "\n" ";" CMAKE_CACHE_VARIABLE_LIST "${CMAKE_CACHE_VARIABLE_LIST}")

    # filter down to the variables
    list(FILTER CMAKE_CACHE_VARIABLE_LIST EXCLUDE REGEX "^$|^//.*$|^\-\-$")

    # Get the configuration types
    set(AVAILABLE_CONFIGURATION_TYPES ${CMAKE_CACHE_VARIABLE_LIST})
    list(FILTER AVAILABLE_CONFIGURATION_TYPES INCLUDE REGEX "^CMAKE_CONFIGURATION_TYPES")
    list(GET AVAILABLE_CONFIGURATION_TYPES 0 AVAILABLE_CONFIGURATION_TYPES)
    string(REGEX REPLACE ".*=" "" AVAILABLE_CONFIGURATION_TYPES "${AVAILABLE_CONFIGURATION_TYPES}")
    string(REPLACE "[:semicolon:]" ";" AVAILABLE_CONFIGURATION_TYPES "${AVAILABLE_CONFIGURATION_TYPES}")
    string(TOUPPER "${AVAILABLE_CONFIGURATION_TYPES}" AVAILABLE_CONFIGURATION_TYPES)

    # Add the current build type if it isn't already there
    string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
    list(FILTER AVAILABLE_CONFIGURATION_TYPES EXCLUDE REGEX ${BUILD_TYPE})
    list(APPEND AVAILABLE_CONFIGURATION_TYPES ${BUILD_TYPE})
    list(SORT AVAILABLE_CONFIGURATION_TYPES)

    # make AVAILABLE_CONFIGURATION_TYPES available to parent
    set(AVAILABLE_CONFIGURATION_TYPES ${AVAILABLE_CONFIGURATION_TYPES} PARENT_SCOPE)
endfunction()

# Sets the CMAKE_PROPERTY_LIST and CMAKE_WHITELISTED_PROPERTY_LIST variables to
# the list of properties
function(get_cmake_property_list)
    # See https://stackoverflow.com/a/44477728/240845
    set(LANGS ASM-ATT ASM ASM_MASM ASM_NASM C CSHARP CUDA CXX FORTRAN HIP ISPC JAVA OBJC OBJCXX RC SWIFT)
    get_available_configuration_types()

    # Get all propreties that cmake supports
    execute_process(COMMAND cmake --help-property-list OUTPUT_VARIABLE CMAKE_PROPERTY_LIST)

    # Convert command output into a CMake list
    string(REGEX REPLACE ";" "\\\\;" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")
    string(REGEX REPLACE "\n" ";" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")

    # Populate "<CONFIG>" with AVAILBLE_CONFIG_TYPES
    set(CONFIG_LINES ${CMAKE_PROPERTY_LIST})
    list(FILTER CONFIG_LINES INCLUDE REGEX "<CONFIG>")
    list(FILTER CMAKE_PROPERTY_LIST EXCLUDE REGEX "<CONFIG>")
    foreach(CONFIG_LINE IN LISTS CONFIG_LINES)
        foreach(CONFIG_VALUE IN LISTS AVAILABLE_CONFIGURATION_TYPES)
            string(REPLACE "<CONFIG>" "${CONFIG_VALUE}" FIXED "${CONFIG_LINE}")
            list(APPEND CMAKE_PROPERTY_LIST ${FIXED})
        endforeach()
    endforeach()

    # Populate "<LANG>" with LANGS
    set(LANG_LINES ${CMAKE_PROPERTY_LIST})
    list(FILTER LANG_LINES INCLUDE REGEX "<LANG>")
    list(FILTER CMAKE_PROPERTY_LIST EXCLUDE REGEX "<LANG>")
    foreach(LANG_LINE IN LISTS LANG_LINES)
        foreach(LANG IN LISTS LANGS)
            string(REPLACE "<LANG>" "${LANG}" FIXED "${LANG_LINE}")
            list(APPEND CMAKE_PROPERTY_LIST ${FIXED})
        endforeach()
    endforeach()

    # no repeats
    list(REMOVE_DUPLICATES CMAKE_PROPERTY_LIST)

    # Fix https://stackoverflow.com/questions/32197663/how-can-i-remove-the-the-location-property-may-not-be-read-from-target-error-i
    list(FILTER CMAKE_PROPERTY_LIST EXCLUDE REGEX "^LOCATION$|^LOCATION_|_LOCATION$")

    list(SORT CMAKE_PROPERTY_LIST)

    # Whitelisted property list for use with interface libraries to reduce warnings
    set(CMAKE_WHITELISTED_PROPERTY_LIST ${CMAKE_PROPERTY_LIST})

    # regex from https://stackoverflow.com/a/51987470/240845
    list(FILTER CMAKE_WHITELISTED_PROPERTY_LIST INCLUDE REGEX "^(INTERFACE|[_a-z]|IMPORTED_LIBNAME_|MAP_IMPORTED_CONFIG_)|^(COMPATIBLE_INTERFACE_(BOOL|NUMBER_MAX|NUMBER_MIN|STRING)|EXPORT_NAME|IMPORTED(_GLOBAL|_CONFIGURATIONS|_LIBNAME)?|NAME|TYPE|NO_SYSTEM_FROM_IMPORTED)$")

    # make the lists available
    set(CMAKE_PROPERTY_LIST ${CMAKE_PROPERTY_LIST} PARENT_SCOPE)
    set(CMAKE_WHITELISTED_PROPERTY_LIST ${CMAKE_WHITELISTED_PROPERTY_LIST} PARENT_SCOPE)
endfunction()

get_cmake_property_list()

function(print_target_properties tgt)
    if(NOT TARGET ${tgt})
      message("There is no target named '${tgt}'")
      return()
    endif()

    get_target_property(target_type ${tgt} TYPE)
    if(target_type STREQUAL "INTERFACE_LIBRARY")
        set(PROPERTIES ${CMAKE_WHITELISTED_PROPERTY_LIST})
    else()
        set(PROPERTIES ${CMAKE_PROPERTY_LIST})
    endif()

    foreach (prop ${PROPERTIES})
        #message ("Checking ${prop}")
        get_property(propval TARGET ${tgt} PROPERTY ${prop} SET)
        if (propval)
            get_target_property(propval ${tgt} ${prop})
            message ("${tgt} ${prop} = ${propval}")
        endif()
    endforeach(prop)
endfunction(print_target_properties)