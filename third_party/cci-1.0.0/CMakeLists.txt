project(SystemC-CCI VERSION "1.1.0" LANGUAGES CXX)
include(SystemCPackage)
add_library(cciapi 
${CMAKE_CURRENT_SOURCE_DIR}/cci_cfg/cci_broker_handle.cpp
${CMAKE_CURRENT_SOURCE_DIR}/cci_cfg/cci_broker_manager.cpp
${CMAKE_CURRENT_SOURCE_DIR}/cci_cfg/cci_originator.cpp
${CMAKE_CURRENT_SOURCE_DIR}/cci_cfg/cci_param_if.cpp
${CMAKE_CURRENT_SOURCE_DIR}/cci_cfg/cci_param_untyped.cpp
${CMAKE_CURRENT_SOURCE_DIR}/cci_cfg/cci_param_untyped_handle.cpp
${CMAKE_CURRENT_SOURCE_DIR}/cci_cfg/cci_report_handler.cpp
${CMAKE_CURRENT_SOURCE_DIR}/cci_core/cci_name_gen.cpp
${CMAKE_CURRENT_SOURCE_DIR}/cci_core/cci_value.cpp
${CMAKE_CURRENT_SOURCE_DIR}/cci_core/cci_value_converter.cpp
${CMAKE_CURRENT_SOURCE_DIR}/cci_utils/broker.cpp
${CMAKE_CURRENT_SOURCE_DIR}/cci_utils/consuming_broker.cpp)
target_include_directories(cciapi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
if(USE_NCSC_SYSTEMC)
	target_compile_definitions(cciapi PUBLIC SC_VERSION_CODE=200301)
endif()
target_link_libraries(cciapi PUBLIC RapidJSON ${SystemC_LIBRARIES}) 
if(NOT TARGET systemc-cci::systemc-cci)
    add_library(systemc-cci::systemc-cci ALIAS cciapi)
endif()

set_target_properties(cciapi PROPERTIES
    VERSION ${PROJECT_VERSION}
    FRAMEWORK FALSE
    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/cci_configuration
)
install(TARGETS cciapi COMPONENT cci
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_ARCHIVE_DIR_MODIFIER}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
        
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/. COMPONENT cci
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.h" # select header files
        )
