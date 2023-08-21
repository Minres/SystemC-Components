cmake_minimum_required (VERSION 3.16)

project (fstlib VERSION 1.0.0)

find_package(ZLIB)
if(ZLIB_FOUND)
	set (SRC fstapi.c fastlz.c)
	if(NOT TARGET lz4::lz4)
    	     list(APPEND SRC lz4.c)
	endif()
	add_library(fstapi ${SRC})
	target_include_directories(fstapi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${ZLIB_INCLUDE_DIRS})
	target_link_libraries(fstapi PRIVATE ${ZLIB_LIBRARIES})
	if(TARGET lz4::lz4)
	     target_link_libraries(fstapi PRIVATE lz4::lz4)
	endif()
	# hack to avoid creating dummy config.h
	target_compile_definitions(fstapi PRIVATE FST_CONFIG_INCLUDE="fst_config.h" FST_DYNAMIC_ALIAS2_DISABLE)
	
	if(MSVC)
	    # define __MINGW32__ to minimize changes to upstream
	    target_compile_definitions(fstapi PRIVATE __MINGW32__  _CRT_SECURE_NO_WARNINGS  FST_DO_MISALIGNED_OPS)
	    target_compile_options(fstapi PRIVATE /wd4244 /wd4267 /wd4146 /wd4996)
	endif()
	
	set_target_properties(fstapi PROPERTIES
	    VERSION ${PROJECT_VERSION}
	    FRAMEWORK FALSE
	    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/fstapi.h
	)
	
	install(TARGETS fstapi
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
endif()
