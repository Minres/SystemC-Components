project(scc-util VERSION 0.0.1 LANGUAGES CXX)

set(SRC util/io-redirector.cpp util/watchdog.cpp)
if(TARGET lz4::lz4)
    list(APPEND SRC util/lz4_streambuf.cpp)
endif()
add_library(${PROJECT_NAME} ${SRC})
add_library(scc::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> # for headers when building
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> # for client in install mode
)
if(TARGET lz4::lz4)
    target_link_libraries(${PROJECT_NAME} PUBLIC lz4::lz4)
endif()

if(CLANG_TIDY_EXE)
    set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
endif() 
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    FRAMEWORK FALSE
    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/scc_util.h
)
install(TARGETS ${PROJECT_NAME} COMPONENT util EXPORT ${PROJECT_NAME}-targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_LIBRARY_DIR_MODIFIER}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_LIBRARY_DIR_MODIFIER}${SCC_ARCHIVE_DIR_MODIFIER}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )

if(NOT DEFINED SCC_CMAKE_CONFIG_DIR)
    set(SCC_CMAKE_CONFIG_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/scc)
endif()

install(EXPORT ${PROJECT_NAME}-targets
        DESTINATION ${SCC_CMAKE_CONFIG_DIR}
        NAMESPACE scc::
        )

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/util COMPONENT util
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.h" # select header files
        )
