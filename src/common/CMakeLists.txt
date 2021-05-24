cmake_minimum_required(VERSION 3.12)

project(scc-util VERSION 0.0.1 LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC util/io-redirector.cpp util/watchdog.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
if(CLANG_TIDY_EXE)
	set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
endif() 
set_target_properties(${PROJECT_NAME} PROPERTIES
    FRAMEWORK FALSE
    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/util/*.h # specify the public headers
)

install(TARGETS ${PROJECT_NAME} 
	          RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
              LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
              ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
             INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/scc-util 
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/scc-util
                      COMPONENT lib)
                           