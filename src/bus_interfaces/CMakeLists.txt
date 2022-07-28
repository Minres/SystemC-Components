project(busses VERSION 0.0.1 LANGUAGES CXX)

set(LIB_SOURCES
    ahb/pin/initiator.cpp
    ahb/pin/target.cpp
    ahb/pe/ahb_initiator.cpp
    ahb/pe/ahb_target.cpp
    apb/pe/apb_initiator.cpp
    apb/pe/apb_target.cpp
    axi/axi_initiator.cpp
    axi/axi_target.cpp
)

add_library(busses ${LIB_SOURCES})
target_include_directories (${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC tlm-interfaces)
target_link_libraries(${PROJECT_NAME} PRIVATE scc-sysc)

if(CLANG_TIDY_EXE)
    set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
endif() 
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    FRAMEWORK FALSE
    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/scc_bus_interfaces.h
)

install(TARGETS ${PROJECT_NAME} COMPONENT busses
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_ARCHIVE_DIR_MODIFIER}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )

foreach(dir IN ITEMS ahb apb atp axi)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${dir} COMPONENT busses
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.h" # select header files
        )
endforeach()        
