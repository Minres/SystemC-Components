project(scv-tr VERSION 0.0.1 LANGUAGES CXX)

include(GNUInstallDirs)

find_package(fmt)

add_library(${PROJECT_NAME} 
	scv-tr/scv_introspection.cpp
	scv-tr/scv_report.cpp
	scv-tr/scv_tr_text.cpp
	scv-tr/scv_tr.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> # for headers when building
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> # for client in install mode
)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" AND TARGET fmt::fmt-header-only)
	target_link_libraries(${PROJECT_NAME} PUBLIC fmt::fmt-header-only)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)
endif()
if(TARGET SystemC::systemc)
	if(USE_CWR_SYSTEMC)
		get_target_property(INCLS SystemC::systemc INTERFACE_INCLUDE_DIRECTORIES)
		get_target_property(DIRS SystemC::systemc INTERFACE_LINK_DIRECTORIES)
		get_target_property(LIBS SystemC::systemc INTERFACE_LINK_LIBRARIES)
    	target_include_directories (${PROJECT_NAME} PUBLIC ${INCLS})    
		target_link_directories(${PROJECT_NAME} PUBLIC ${DIRS})
		target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS})
	else()
		target_link_libraries(${PROJECT_NAME} PUBLIC SystemC::systemc)
	endif()
else()
	target_compile_definitions(${PROJECT_NAME} PUBLIC ${SystemC_DEFINITIONS})
	target_include_directories (${PROJECT_NAME} PUBLIC ${SystemC_INCLUDE_DIRS})
	if(SystemC_LIBRARY_DIRS)
		target_link_directories(${PROJECT_NAME} PUBLIC ${SystemC_LIBRARY_DIRS})
	endif()
	target_link_libraries(${PROJECT_NAME} PUBLIC ${SystemC_LIBRARIES})
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    FRAMEWORK FALSE
    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/scv-tr.h
    CXX_CLANG_TIDY ""
)

install(TARGETS ${PROJECT_NAME} COMPONENT scv-tr EXPORT ${PROJECT_NAME}-targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_LIBRARY_DIR_MODIFIER}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_LIBRARY_DIR_MODIFIER}${SCC_ARCHIVE_DIR_MODIFIER}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
        
set(SCV_TR_CMAKE_CONFIG_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/scv-tr)
include(CMakePackageConfigHelpers)

install(EXPORT ${PROJECT_NAME}-targets
        DESTINATION ${SCV_TR_CMAKE_CONFIG_DIR})

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/scv-tr-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/scv-tr-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/scv-tr-config.cmake
    INSTALL_DESTINATION ${SCV_TR_CMAKE_CONFIG_DIR}
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/scv-tr-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/scv-tr-config-version.cmake
    DESTINATION ${SCV_TR_CMAKE_CONFIG_DIR})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scv-tr COMPONENT scv-tr
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.h" # select header files
        )
