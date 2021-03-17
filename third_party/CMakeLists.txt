cmake_minimum_required(VERSION 3.12)

project(jsoncpp VERSION 0.0.1 LANGUAGES CXX)

###############################################################################
# build a separate JSON library for the use in non-systemc based systems
###############################################################################
add_library(jsoncpp STATIC json/jsoncpp.cpp)
target_include_directories (jsoncpp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

FILE(GLOB JsonLibHeaders json/*.h)
set_target_properties(jsoncpp PROPERTIES
    VERSION 1.8.3
    FRAMEWORK FALSE
    PUBLIC_HEADER "${JsonLibHeaders}" # specify the public headers
)

install(TARGETS jsoncpp
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/json
        )                 