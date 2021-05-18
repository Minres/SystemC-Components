cmake_minimum_required(VERSION 3.12)

project(busses VERSION 0.0.1 LANGUAGES CXX)

set(LIB_SOURCES
    ahb/tlm_ahb_bfm_initiator.cpp
    ahb/tlm_ahb_bfm_target.cpp
    ahb/ahb_initiator.cpp
    apb/apb_initiator.cpp
    axi/axi_initiator.cpp
 	ahb/ahb_target.cpp
    apb/apb_target.cpp
    axi/axi_target.cpp
)

add_library(busses ${LIB_SOURCES})
target_include_directories (busses PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(busses PRIVATE tlm-interfaces scc-sysc)

install(TARGETS busses COMPONENT busses
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
        )
        
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" COMPONENT busses
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.h" # select header files
        )
        