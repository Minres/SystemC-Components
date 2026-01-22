project(interfaces VERSION 0.0.1 LANGUAGES CXX)

set(LIB_SOURCES
	apb/apb_tlm.cpp
	ahb/ahb_tlm.cpp
    ahb/pe/ahb_initiator.cpp
    ahb/pe/ahb_target.cpp
    apb/pe/apb_initiator.cpp
    apb/pe/apb_target.cpp
    axi/axi_initiator.cpp
    axi/axi_target.cpp
	ocp/ocp_tlm.cpp
    tilelink/tl_tlm.cpp
    cxs/cxs_tlm.cpp
    eth/eth_tlm.cpp
)

add_library(${PROJECT_NAME} ${LIB_SOURCES})
add_library(scc::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
add_library(busses ALIAS ${PROJECT_NAME})
#TODO: remove alias
add_library(scc::busses ALIAS ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> # for headers when building
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> # for client in install mode
)
target_link_libraries(${PROJECT_NAME} PUBLIC tlm-interfaces)
target_link_libraries(${PROJECT_NAME} PRIVATE scc-sysc)

if(CLANG_TIDY_EXE)
    set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
endif() 
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    FRAMEWORK FALSE
    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/scc_interfaces.h
)

install(TARGETS ${PROJECT_NAME} COMPONENT busses EXPORT ${PROJECT_NAME}-targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_LIBRARY_DIR_MODIFIER}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_LIBRARY_DIR_MODIFIER}${SCC_ARCHIVE_DIR_MODIFIER}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )

install(EXPORT ${PROJECT_NAME}-targets
        DESTINATION ${SCC_CMAKE_CONFIG_DIR}
        NAMESPACE scc::
        )

foreach(dir IN ITEMS ahb apb axi cxs obi ocp spi tilelink)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${dir} COMPONENT busses
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.h" # select header files
        )
endforeach()        
