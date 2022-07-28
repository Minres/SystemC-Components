project(scv-tr VERSION 0.0.1 LANGUAGES CXX)

include(GNUInstallDirs)

include(SystemCPackage)

add_library(${PROJECT_NAME} 
	scv-tr/scv_introspection.cpp
	scv-tr/scv_report.cpp
	scv-tr/scv_tr_text.cpp
	scv-tr/scv_tr.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/fmt_8.0)
target_compile_definitions(${PROJECT_NAME} PRIVATE FMT_HEADER_ONLY)

target_link_libraries(${PROJECT_NAME} PUBLIC ${FMT_TARGET})
if(TARGET SystemC::systemc)
	if(USE_CWR_SYSTEMC)
		get_target_property(INCLS SystemC::systemc INTERFACE_INCLUDE_DIRECTORIES)
		get_target_property(DIRS SystemC::systemc INTERFACE_LINK_DIRECTORIES)
		get_target_property(LIBS SystemC::systemc INTERFACE_LINK_LIBRARIES)
    	target_include_directories (${PROJECT_NAME} PUBLIC ${INCLS})    
		target_link_directories(${PROJECT_NAME} PUBLIC ${DIRS})
		target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS})
	else()
		target_link_libraries(${PROJECT_NAME} PUBLIC SystemC::systemc)
	endif()
else()
	target_compile_definitions(${PROJECT_NAME} PUBLIC ${SystemC_DEFINITIONS})
	target_include_directories (${PROJECT_NAME} PUBLIC ${SystemC_INCLUDE_DIRS})
	if(SystemC_LIBRARY_DIRS)
		target_link_directories(${PROJECT_NAME} PUBLIC ${SystemC_LIBRARY_DIRS})
	endif()
	target_link_libraries(${PROJECT_NAME} PUBLIC ${SystemC_LIBRARIES})
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    FRAMEWORK FALSE
    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/scv-tr.h
)

install(TARGETS ${PROJECT_NAME} COMPONENT scv-tr
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_ARCHIVE_DIR_MODIFIER}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
        
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scv-tr COMPONENT scv-tr
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.h" # select header files
        )
