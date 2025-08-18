project(SystemC-CCI VERSION "1.1.0" LANGUAGES CXX)

add_library(cciapi 
                        cci/core/cci_value_converter.cpp
                        cci/core/cci_name_gen.cpp
                        cci/core/cci_value.cpp
                        cci/utils/consuming_broker.cpp
                        cci/utils/broker.cpp
                        cci/cfg/cci_report_handler.cpp
                        cci/cfg/cci_param_if.cpp
                        cci/cfg/cci_broker_manager.cpp
                        cci/cfg/cci_param_untyped.cpp
                        cci/cfg/cci_param_untyped_handle.cpp
                        cci/cfg/cci_originator.cpp
                        cci/cfg/cci_broker_handle.cpp)
if(USE_NCSC_SYSTEMC OR USE_CWR_SYSTEMC)
	target_compile_definitions(cciapi PUBLIC SC_VERSION_CODE=200301)
endif()
target_include_directories(cciapi PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> # for headers when building
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> # for client in install mode
)
if(USE_CWR_SYSTEMC OR USE_NCSC_SYSTEMC)
	get_target_property(SC_COMPILE_OPTIONS ${SystemC_LIBRARIES} INTERFACE_COMPILE_OPTIONS)
	if(SC_COMPILE_OPTIONS) 
	    target_compile_definitions(cciapi PUBLIC ${SC_COMPILE_OPTIONS})
	else()
	    target_compile_definitions(cciapi PUBLIC ${SystemC_DEFINITIONS})
	endif()
	get_target_property(SC_INCLUDE_DIRS ${SystemC_LIBRARIES} INTERFACE_INCLUDE_DIRECTORIES)
	if(SC_INCLUDE_DIRS) 
	    target_include_directories(cciapi PUBLIC ${SC_INCLUDE_DIRS})
	else()
	    target_include_directories(cciapi PUBLIC ${SystemC_INCLUDE_DIRS})
	endif()
else()
	target_link_libraries(cciapi PUBLIC SystemC::systemc) 
endif()
target_link_libraries(cciapi PUBLIC RapidJSON) 

if(NOT TARGET systemc-cci::systemc-cci)
    add_library(systemc-cci::systemc-cci ALIAS cciapi)
endif()

set_target_properties(cciapi PROPERTIES
    VERSION ${PROJECT_VERSION}
    FRAMEWORK FALSE
    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/cci_configuration
    CXX_CLANG_TIDY ""
)

set(CCIAPI_CMAKE_CONFIG_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/cciapi)
install(TARGETS cciapi COMPONENT cci EXPORT cciapi-targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_LIBRARY_DIR_MODIFIER}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_LIBRARY_DIR_MODIFIER}${SCC_ARCHIVE_DIR_MODIFIER}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )

install(EXPORT cciapi-targets
        DESTINATION ${CCIAPI_CMAKE_CONFIG_DIR})
        
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/cciapi-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/cciapi-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cciapi-config.cmake
    INSTALL_DESTINATION ${CCIAPI_CMAKE_CONFIG_DIR}
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/cciapi-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cciapi-config-version.cmake
    DESTINATION ${CCIAPI_CMAKE_CONFIG_DIR})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/. COMPONENT cci
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.h" # select header files
        )
