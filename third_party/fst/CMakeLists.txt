cmake_minimum_required (VERSION 3.20)

project (fstapi VERSION 1.0.0)

find_package(ZLIB)
if(TARGET ZLIB::ZLIB)
	set(SRC fstapi.c fastlz.c)
	if(NOT TARGET lz4::lz4)
    	     list(APPEND SRC lz4.c)
	endif()
	add_library(fstapi ${SRC})
	target_include_directories(fstapi PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> # for headers when building
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> # for client in install mode
	)
	target_link_libraries(fstapi PRIVATE ZLIB::ZLIB)
	if(TARGET lz4::lz4)
	     target_link_libraries(fstapi PRIVATE lz4::lz4)
	endif()
	# hack to avoid creating dummy config.h	
	if(MSVC)
	    # define __MINGW32__ to minimize changes to upstream
	    target_compile_definitions(fstapi PRIVATE FST_CONFIG_INCLUDE="fstapi.h" __MINGW32__  _CRT_SECURE_NO_WARNINGS  FST_DO_MISALIGNED_OPS)
	    target_compile_options(fstapi PRIVATE /wd4244 /wd4267 /wd4146 /wd4996)
	else()
		target_compile_definitions(fstapi PRIVATE FST_CONFIG_INCLUDE="fstapi.h")
	endif()
	
	set_target_properties(fstapi PROPERTIES
	    VERSION ${PROJECT_VERSION}
	    FRAMEWORK FALSE
	    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/fstapi.h
	)
	
	install(TARGETS fstapi EXPORT fstapi-targets
	        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_LIBRARY_DIR_MODIFIER}
	        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_LIBRARY_DIR_MODIFIER}${SCC_ARCHIVE_DIR_MODIFIER}
	        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/fst
	)
        
	install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # target directory
	        FILES_MATCHING # install only matched files
	        PATTERN "*.h" # select header files
	        )

    set(FST_CMAKE_CONFIG_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/fstapi)
    include(CMakePackageConfigHelpers)
    
    install(EXPORT fstapi-targets
        DESTINATION ${FST_CMAKE_CONFIG_DIR})
    
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/fstapi-config-version.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
    )
    
    configure_package_config_file(
        ${CMAKE_CURRENT_LIST_DIR}/fstapi-config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/fstapi-config.cmake
        INSTALL_DESTINATION ${FST_CMAKE_CONFIG_DIR}
    )
    
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/fstapi-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/fstapi-config-version.cmake
        DESTINATION ${FST_CMAKE_CONFIG_DIR})
    endif()
