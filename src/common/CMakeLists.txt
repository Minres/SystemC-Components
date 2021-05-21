cmake_minimum_required(VERSION 3.12)

project(scc-util VERSION 0.0.1 LANGUAGES CXX)

add_library(scc-util util/io-redirector.cpp util/watchdog.cpp)

target_include_directories(scc-util PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(scc-util PROPERTIES
    VERSION ${PROJECT_VERSION}
    FRAMEWORK FALSE
    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/scc_util.h
)
install(TARGETS scc-util COMPONENT util
	    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/util COMPONENT util
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.h" # select header files
        )
                           