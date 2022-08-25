project(scc-util VERSION 0.0.1 LANGUAGES CXX)

set(SRC util/io-redirector.cpp util/watchdog.cpp)
if(TARGET lz4::lz4 OR TARGET CONAN_PKG::lz4)
    list(APPEND SRC util/lz4_streambuf.cpp)
endif()
add_library(${PROJECT_NAME} ${SRC})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
if(TARGET lz4::lz4)
    target_link_libraries(${PROJECT_NAME} PUBLIC lz4::lz4)
elseif(TARGET CONAN_PKG::lz4)
    target_link_libraries(${PROJECT_NAME} PUBLIC CONAN_PKG::lz4)
endif()

if(CLANG_TIDY_EXE)
    set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
endif() 
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    FRAMEWORK FALSE
    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/scc_util.h
)
install(TARGETS ${PROJECT_NAME} COMPONENT util
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_ARCHIVE_DIR_MODIFIER}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/util COMPONENT util
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.h" # select header files
        )
