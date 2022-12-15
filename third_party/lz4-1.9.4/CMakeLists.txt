project(lz4 VERSION "1.9.4" LANGUAGES C)
add_library(lz4 lz4.c lz4file.c lz4frame.c lz4hc.c xxhash.c)
target_include_directories(lz4 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(lz4 PROPERTIES
    VERSION ${PROJECT_VERSION}
    FRAMEWORK FALSE
    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/lz4.h
)
install(TARGETS lz4 COMPONENT lz4
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_ARCHIVE_DIR_MODIFIER}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
        
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/. COMPONENT lz4
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.h" # select header files
        )

if(NOT TARGET lz4::lz4)
        add_library(lz4::lz4 INTERFACE IMPORTED)
        if(lz4_INCLUDE_DIRS)
            set_target_properties(lz4::lz4 PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR})")
        endif()
        set_property(TARGET lz4::lz4 PROPERTY INTERFACE_LINK_LIBRARIES "lz4")
        #set_property(TARGET lz4::lz4 PROPERTY INTERFACE_COMPILE_DEFINITIONS ${lz4_COMPILE_DEFINITIONS})
        #set_property(TARGET lz4::lz4 PROPERTY INTERFACE_COMPILE_OPTIONS "${lz4_COMPILE_OPTIONS_LIST}")
endif()
        