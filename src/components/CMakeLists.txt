cmake_minimum_required(VERSION 3.12)

project(components VERSION 0.0.1 LANGUAGES CXX)

add_library(components INTERFACE)
target_include_directories (components INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(components INTERFACE scc-sysc)

set_target_properties(components PROPERTIES
    # VERSION ${PROJECT_VERSION}
    # FRAMEWORK FALSE
    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/scc_components.h
)

install(TARGETS components COMPONENT components
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
        
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scc COMPONENT components
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.h" # select header files
        )
        