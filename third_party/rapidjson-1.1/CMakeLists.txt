cmake_minimum_required (VERSION 3.16)

project(RapidJSON VERSION "1.1.0" LANGUAGES CXX)

set(${PROJECT_NAME}_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include> # for headers when building
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> # for client in install mode
)

if(CMAKE_CXX_CPPCHECK)
    target_compile_definitions(${PROJECT_NAME} INTERFACE RAPIDJSON_ENDIAN=RAPIDJSON_LITTLEENDIAN)
endif()
	
set_target_properties(RapidJSON PROPERTIES
    VERSION ${PROJECT_VERSION}
    FRAMEWORK FALSE
    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/include/rapidjson/rapidjson.h
)

install(TARGETS RapidJSON EXPORT RapidJSON-targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_LIBRARY_DIR_MODIFIER}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_LIBRARY_DIR_MODIFIER}${SCC_ARCHIVE_DIR_MODIFIER}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
    
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/rapidjson
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.h" # select header files
        )


set(RAPIDJSON_CMAKE_CONFIG_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/RapidJSON)
include(CMakePackageConfigHelpers)

install(EXPORT RapidJSON-targets
        DESTINATION ${RAPIDJSON_CMAKE_CONFIG_DIR})

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/RapidJSON-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/rapidjson-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/rapidjson-config.cmake
    INSTALL_DESTINATION ${RAPIDJSON_CMAKE_CONFIG_DIR}
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/rapidjson-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/RapidJSON-config-version.cmake
    DESTINATION ${RAPIDJSON_CMAKE_CONFIG_DIR})

