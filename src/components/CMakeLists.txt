cmake_minimum_required(VERSION 3.12)

project(components VERSION 0.0.1 LANGUAGES CXX)

add_library(components INTERFACE)
target_include_directories (components INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(components INTERFACE sysc)

set_target_properties(components PROPERTIES
    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/*
)

install(TARGETS components
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/components
        )