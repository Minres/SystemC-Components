project(lz4 VERSION "1.9.4" LANGUAGES C)
add_library(lz4 lz4.c lz4file.c lz4frame.c lz4hc.c xxhash.c)
target_include_directories(lz4 PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> # for headers when building
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> # for client in install mode
)

if(NOT TARGET lz4::lz4)
    add_library(lz4::lz4 ALIAS lz4)
endif()

set_target_properties(lz4 PROPERTIES
    VERSION ${PROJECT_VERSION}
    FRAMEWORK FALSE
    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/lz4.h
)
install(TARGETS lz4 COMPONENT lz4 EXPORT lz4-targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_ARCHIVE_DIR_MODIFIER}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
        
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/. COMPONENT lz4
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.h" # select header files
        )

set(LZ4_CMAKE_CONFIG_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/lz4)
include(CMakePackageConfigHelpers)

install(EXPORT lz4-targets
    DESTINATION ${LZ4_CMAKE_CONFIG_DIR})

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/lz4-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/lz4-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/lz4-config.cmake
    INSTALL_DESTINATION ${LZ4_CMAKE_CONFIG_DIR}
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/lz4-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/lz4-config-version.cmake
    DESTINATION ${LZ4_CMAKE_CONFIG_DIR})
