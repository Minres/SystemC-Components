project(scc-util VERSION 0.0.1 LANGUAGES CXX)

set(SRC util/io-redirector.cpp util/watchdog.cpp util/ihex.cpp)
if(TARGET lz4::lz4)
    list(APPEND SRC util/lz4_streambuf.cpp)
endif()
if(TARGET elfio::elfio)
    list(APPEND SRC util/elf.cpp)
endif()
if(TARGET ZLIB::ZLIB)
    list(APPEND SRC util/gzip_streambuf.cpp)
endif()
if(TARGET BZip2::BZip2)
    list(APPEND SRC util/bzip2_streambuf.cpp)
endif()
if(TARGET zstd::libzstd_static)
    list(APPEND SRC util/zstd_streambuf.cpp)
endif()
if(TARGET LibLZMA::LibLZMA)
    list(APPEND SRC util/xz_streambuf.cpp)
endif()
add_library(${PROJECT_NAME} ${SRC})
add_library(scc::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> # for headers when building
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> # for client in install mode
)
if( NOT MSVC)
    # rigtorp/SPSCQueue.h uses std::hardware_destructive_interference_size which is HW dependend. GCC>14  warns about it
    target_compile_options(${PROJECT_NAME} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-Wno-interference-size>)
endif()
if(TARGET lz4::lz4)
    target_link_libraries(${PROJECT_NAME} PUBLIC lz4::lz4)
endif()
if(TARGET elfio::elfio)
    target_link_libraries(${PROJECT_NAME} PRIVATE elfio::elfio)
endif()
if(TARGET ZLIB::ZLIB)
    target_link_libraries(${PROJECT_NAME} PUBLIC ZLIB::ZLIB)
endif()
if(TARGET BZip2::BZip2)
    target_link_libraries(${PROJECT_NAME} PUBLIC BZip2::BZip2)
endif()
if(TARGET zstd::libzstd_static)
    target_link_libraries(${PROJECT_NAME} PUBLIC zstd::libzstd_static)
endif()
if(TARGET LibLZMA::LibLZMA)
    target_link_libraries(${PROJECT_NAME} PUBLIC LibLZMA::LibLZMA)
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
