project(components VERSION ${scc_VERSION} LANGUAGES CXX)

find_package(Boost REQUIRED COMPONENTS thread chrono)
find_package(flatbuffers CONFIG QUIET)

if(TARGET flatbuffers::flatbuffers)
        # flatbuffers scheme to be compiled by flac binary
        set(FB_SCHEMA "${CMAKE_CURRENT_LIST_DIR}/ipc.fbs")
        # Location of the generated files. I like to use CMAKE_CURRENT_BINARY_DIR
        # So it will not be in the source code tree.
        set(FB_OUTDIR "${CMAKE_CURRENT_BINARY_DIR}/fbs/")
        # This is an utilitary function that is available in flatbuffer cmake integration
        # fbschemas is a new target that will be created with the generated file
        build_flatbuffers("${FB_SCHEMA}" "" fbschemas "" "${FB_OUTDIR}" "" "")

        add_library(${PROJECT_NAME} scc/tcp4tlm_bridge.cpp)
        add_library(scc::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
        target_include_directories(${PROJECT_NAME} PUBLIC
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> # for headers when building
                $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> # for client in install mode
        )
        target_include_directories(${PROJECT_NAME} PRIVATE 
                $<BUILD_INTERFACE:${flatbuffers_INCLUDE_DIR}> # for headers when building
                $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR})
        target_link_libraries(${PROJECT_NAME} PUBLIC scc-sysc scc-util)
        target_link_libraries(${PROJECT_NAME} PUBLIC flatbuffers::flatbuffers)
        if(TARGET Boost::thread)
                target_link_libraries(${PROJECT_NAME} PUBLIC Boost::thread Boost::chrono)
        else()
                target_link_libraries(${PROJECT_NAME} PUBLIC
                        ${Boost_THREAD_LIBRARY}
                        ${Boost_CHRONO_LIBRARY}
                )
        endif()
else()
        add_library(${PROJECT_NAME} INTERFACE)
        add_library(scc::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
        target_include_directories(${PROJECT_NAME} INTERFACE
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> # for headers when building
                $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> # for client in install mode
        )
        target_link_libraries(${PROJECT_NAME} INTERFACE scc-sysc)

endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
        # VERSION ${PROJECT_VERSION}
        # FRAMEWORK FALSE
        PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/scc_components.h
)

install(TARGETS ${PROJECT_NAME} COMPONENT components EXPORT ${PROJECT_NAME}-targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_LIBRARY_DIR_MODIFIER}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}${SCC_LIBRARY_DIR_MODIFIER}${SCC_ARCHIVE_DIR_MODIFIER}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT ${PROJECT_NAME}-targets
        DESTINATION ${SCC_CMAKE_CONFIG_DIR}
        NAMESPACE scc::
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scc COMPONENT components
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.h" # select header files
        PATTERN "*.fbs" # install FlatBuffers schemas
)
