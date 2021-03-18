cmake_minimum_required(VERSION 3.12)

project("scc-util" VERSION 0.0.1 LANGUAGES CXX)

add_library(scc-util STATIC util/io-redirector.cpp util/watchdog.cpp)

target_include_directories(scc-util PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(scc-util PROPERTIES
    FRAMEWORK FALSE
    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/util/*.h # specify the public headers
)

install(TARGETS scc-util 
	          RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
              LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
              ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
             INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/scc-util 
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/scc-util
                      COMPONENT lib)
                           