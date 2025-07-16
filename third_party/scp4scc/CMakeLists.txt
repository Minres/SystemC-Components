project(scp-reporting VERSION "1.0.0" LANGUAGES CXX)

add_library(${PROJECT_NAME} scp/report.cpp)
add_library(scp::reporting ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> # for headers when building
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> # for client in install mode
)
target_link_libraries(${PROJECT_NAME} PUBLIC scc::sysc) 

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    FRAMEWORK FALSE
    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}
    CXX_CLANG_TIDY ""
)

set(SCP_CMAKE_CONFIG_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/scp)
install(TARGETS ${PROJECT_NAME} EXPORT scp-targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_LIBRARY_DIR_MODIFIER}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_LIBRARY_DIR_MODIFIER}${SCC_ARCHIVE_DIR_MODIFIER}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )

install(EXPORT scp-targets
        DESTINATION ${SCP_CMAKE_CONFIG_DIR})
        
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/scp-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/scp-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/scp-config.cmake
    INSTALL_DESTINATION ${SCP_CMAKE_CONFIG_DIR}
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/scp-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/scp-config-version.cmake
    DESTINATION ${SCP_CMAKE_CONFIG_DIR})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/. COMPONENT cci
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.h" # select header files
        )
